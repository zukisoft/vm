cmd_/home/djp952/external-kernelheaders/linux-4.4.5//include/uapi/.install := /bin/bash scripts/headers_install.sh /home/djp952/external-kernelheaders/linux-4.4.5//include/uapi ./include/uapi ; /bin/bash scripts/headers_install.sh /home/djp952/external-kernelheaders/linux-4.4.5//include/uapi ./include ; /bin/bash scripts/headers_install.sh /home/djp952/external-kernelheaders/linux-4.4.5//include/uapi ./include/generated/uapi ; for F in ; do echo "\#include <asm-generic/$$F>" > /home/djp952/external-kernelheaders/linux-4.4.5//include/uapi/$$F; done; touch /home/djp952/external-kernelheaders/linux-4.4.5//include/uapi/.install