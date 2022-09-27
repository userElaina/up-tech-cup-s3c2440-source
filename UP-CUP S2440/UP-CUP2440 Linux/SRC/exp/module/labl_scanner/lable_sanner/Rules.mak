TOPDIR= ..
#CROSS = armv4l-unknown-linux-
CROSS = arm-linux-
CC= ${CROSS}gcc

LDFLAGS += -static  
EXTRA_LIBS += 
EXP_INSTALL = install -m 755 
INSTALL_DIR = ./sqlite
