TPDIR= ..
CROSS = arm-linux-
#CROSS = armv4l-unknown-linux-
CC= ${CROSS}gcc

LDFLAGS += -static  
EXTRA_LIBS += 
EXP_INSTALL = install -m 755 
INSTALL_DIR = ../bin
