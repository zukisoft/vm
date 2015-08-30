//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef __PROCESSMODEL_H_
#define __PROCESSMODEL_H_
#pragma once

#include <map>
#include <memory>
#include "LinuxException.h"
#include "_VmOld.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ProcessModel
//
// Implements the standard parent/child relationships for pid-based objects
// like sessions, process groups, etc.
//
// TODO: need more words here when fleshed out
//
// NAMES: ContainerOf<>, ChildOf<> ??

namespace ProcessModel {

// Forward Declarations
template<class _childtype> class Parent;

// Child
//
template<class _derived>
class Child
{
friend class Parent<_derived>;
public:

	virtual ~Child()
	{
	}

protected:

	// Make note that derived object must accept uapi::pid_t as first ctor argument
	Child(uapi::pid_t pid) : m_pid(pid)
	{
	}

private:

	Child(const Child&)=delete;
	Child& operator=(const Child&)=delete;

	//// accesed by Parent<> only, tag with an underscore or something to prevent name clashes
	//// or just access the mvars directly since Parent<> is a friend class
	////

	__declspec(property(get=getOwned)) bool Owned;
	bool getOwned(void) const { return !m_owner.expired(); }

	__declspec(property(get=getPid)) uapi::pid_t Pid;
	uapi::pid_t getPid(void) const { return m_pid; }

	void SetOwner(const std::shared_ptr<Parent<_derived>>& value) 
	{ 
		_ASSERTE(value);
		m_owner = value; 
	}

	//-------------------------------------------------------------------------
	// Member Variables

	const uapi::pid_t					m_pid;		// PID (immutable)
	std::weak_ptr<Parent<_derived>>		m_owner;	// Parent object reference
};

// Parent
//
// words
template<class _childtype>
class Parent
{
public:

	// Destructor
	//
	virtual ~Parent()
	{
		auto vm = m_vm.lock();			// Parent virtual machine instance

		// There should not be any children left during destruction
		_ASSERTE(m_children.size() == 0);

		// Release the PIDs for any children that still exist in the collection
		for(const auto& iterator : m_children)
			if((iterator.first != m_pid) && vm) vm->ReleasePID(iterator.first);
	}

	//-------------------------------------------------------------------------
	// Member Functions

	// Add
	//
	// Constructs a new child object in-place within the collection, note that
	// the child must have uapi::pid_t as its first constructor argument
	template<class... _args>
	std::shared_ptr<_childtype> Add(uapi::pid_t pid, _args&&... args)
	{
		child_map_lock_t::scoped_lock_write writer(m_childrenlock);

		// Construct a new shared_ptr<> for the child object in-place
		auto emplaced = m_children.emplace(pid, std::forward<_args>(args)...);
		if(!emplaced.second) throw LinuxException(LINUX_ESRCH);		// todo: inner exception

		// Child was successfully added, set the parent reference and return
		emplaced.first->second->SetOwner(this->SharedPointer);
		return emplaced.first->second;
	}

	// Attach
	//
	// Attaches an existing unowned child object to this collection
	void Attach(const std::shared_ptr<_childtype>& child)
	{
		// Child object must not be owned by another parent container
		_ASSERTE(child && !child->Owned);
		if(child->Owned) throw LinuxException(LINUX_EPERM);			// todo: inner exception

		// Attempt to insert the child into this parent container
		auto emplaced = m_children.emplace(child->Pid, child);
		if(!emplaced.second) throw LinuxException(LINUX_ESRCH);		// todo: inner exception

		// Child was successfully attached, change its owner to this container
		child->SetOwner(this->SharedPointer);
	}

	// Detach
	//
	// Detaches a child object from this collection
	std::shared_ptr<_childtype> Detach(uapi::pid_t pid)
	{
		child_map_lock_t::scoped_lock_write writer(m_childrenlock);

		// Attempt to locate the specified child in the collection
		const auto& iterator = m_children.find(pid);
		if(iterator == m_children.end()) throw LinuxException(LINUX_ESRCH);	// todo: inner exception

		// Copy out a new shared_ptr<> for the child object and remove from the collection
		auto result = iterator->second;
		m_children.erase(iterator);

		result->SetOwner(nullptr);			// Child object is no longer owned
		return result;						// Return acquired shared_ptr<>
	}

	// Remove
	//
	// Removes a child object from the collection
	void Remove(uapi::pid_t pid)
	{
		child_map_lock_t::scoped_lock_write writer(m_childrenlock);
		if(m_children.erase(pid) == 0) throw LinuxException(LINUX_ESRCH);		// todo: inner exception

		// If the child object is not the leader of this container, release the PID
		auto vm = m_vm.lock();
		if((pid != m_pid) && vm) vm->ReleasePID(pid);
	}

	// Swap
	//
	// Swaps ownership of a child object with another parent container
	void Swap(uapi::pid_t pid, const std::shared_ptr<Parent<_childtype>>& rhs)
	{
		// The lead child cannot be swapped into another parent container
		if(pid == m_pid) throw LinuxException(LINUX_EPERM);			// todo: inner exception

		// Both collections need to be locked for exclusive access
		child_map_lock_t::scoped_lock_write lhs_writer(m_childrenlock);
		child_map_lock_t::scoped_lock_write rhs_writer(rhs->m_childrenlock);

		// Get a reference to the left-hand child in this parent container
		const auto& leftchild = m_children.find(pid);
		if(leftchild == m_children.end()) throw LinuxException(LINUX_ESRCH);	// todo: inner exception

		// Attempt to emplace a new shared_ptr<> for the child in rhs
		auto emplaced = rhs->m_children.emplace(pid, leftchild.second);
	}

	//-------------------------------------------------------------------------
	// Properties

	// Leader
	//
	// Gets a reference to the 'lead' child object, or null if there is no leader
	__declspec(property(get=getLeader)) std::shared_ptr<_childtype> Leader;
	std::shared_ptr<_childtype> getLeader(void)
	{
		child_map_lock_t::scoped_lock_read reader(m_childrenlock);

		// Attempt to locate a child in the collection that has the same PID
		// as this parent container, that object is the 'leader'
		const auto& iterator = m_children.find(m_pid);
		return (iterator == m_children.end()) ? nullptr : iterator->second;
	}

protected:

	// todo: not used; idea is to make .SharedPointer easier
	using containerref_t = Parent<_childtype>;

	// Instance Constructor
	//
	Parent(const std::shared_ptr<_VmOld>& vm, uapi::pid_t pid) : m_vm(vm), m_pid(pid)
	{
	}

	// SharedPointer (pure virtual)
	//
	// Acquires a shared pointer for the derived object
	__declspec(property(get=getSharedPointer)) std::shared_ptr<Parent<_childtype>> SharedPointer;
	virtual std::shared_ptr<Parent<_childtype>> getSharedPointer(void) = 0;

private:

	Parent(const Parent&)=delete;
	Parent& operator=(const Parent&)=delete;

	// child_map_t
	//
	// Collection of owned child objects
	using child_map_t = std::map<uapi::pid_t, std::shared_ptr<_childtype>>;

	// child_map_lock_t
	//
	// Synchronization object for the child_map_t collection
	using child_map_lock_t = sync::reader_writer_lock;

	//-------------------------------------------------------------------------
	// Member Variables

	const std::weak_ptr<_VmOld>	m_vm;			// Virtual machine instance
	const uapi::pid_t					m_pid;			// Object identifier
	child_map_t							m_children;		// Owned child objects
	child_map_lock_t					m_childrenlock;	// Synchronization object
};

//-----------------------------------------------------------------------------

}	// namespace ProcessModel

#pragma warning(pop)

#endif	// __PROCESSMODEL_H_
