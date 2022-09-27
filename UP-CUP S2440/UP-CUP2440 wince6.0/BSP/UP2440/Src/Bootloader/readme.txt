
Overview of Samsung SmartMedia (NAND) layout when using partitioned flash images:

NOR:

+-------+
|       |
| EBOOT |
|       |
+-------+

NAND:

Block numbers:
0             3                                                             FFF
+-------+-----+---------------------------------------------------------------+
| STEP  |     |                                                               |
| LOADR | IPL |                      PARTITIONS                               |
|       |     |                                                               |
+-------+-----+---------------------------------------------------------------+


Typical NAND "Image Update" flash layout:

Block numbers:
0             3                                                             FFF
+-------+-----+-+------+----+-------------------------------------------------+
| STEP  |     |M|      |    |                                                 |
| LOADR | IPL |B| ULDR | NK |                  SYSTEM                         |
|       |     |R|      |    |                                                 |
+-------+-----+-+------+----+-------------------------------------------------+

* EBOOT is the "standard" Ethernet bootloader and it can download images into
  RAM and optionally write them into NAND flash.  EBOOT is used to download the 
  entire "Image Update" disk image (IPL, MBR, NK, SYSTEM).  EBOOT lives in the 
  Samsung's NOR flash part.
 
* STEP LOADR is the Samsung S3C2440 4KB "Steppingstone" loader.  This loader is used
  when booting directly off of the SmartMedia card (as opposed to booting off the NOR
  part first).  At reset, the CPU streams the first 4KB (known to be good) of the 
  SmartMedia card into an internal RAM buffer.  The code executes internally, configures
  the CPU/board, and bootstraps the remainder of the image.
   
* IPL is the "initial program loader" defined as part of the "Image Update" solution.
  The IPL is tasked with loading either the ULDR or NK depending on whether the platform 
  were in an update mode or in a normal boot mode.
  
* MBR is the "master boot record" and models the well-defined PC MBR architecture.

* ULDR is the "update loader" defined as part of the "Image Update" solution.  It's 
  tasked with applying any valid updates to the NK and/or SYSTEM (and/or reserved)
  partitions.  The ULDR may be compressed since it must run out of RAM.
  
* NK is the core of the primary OS image and is either compressed or not.  The NK 
  partition contains enough code to bootstrap the system and get the filesystem drivers 
  loaded in order to view the remainder of the OS image which is stored in the SYSTEM partition.
  
* SYSTEM is a filesystem-managed binary partition where the balance of the OS image
  is stored (the other part being stored in the NK partition).

Preparing the SmartMedia for booting an "Image Update" image:
-------------------------------------------------------------

1. Make sure the bootloader on the device is version 2.0 or greater (look for the version to be
   displayed at boot time over the debug serial port).
   
2. Ensure the SmartMedia card is inserted in the card socket and power-on the Samsung board.  Stop 
   the bootloader in the boot menu (press the space bar in the terminal emulator program) and format
   the SmartMedia card using the "format" menu option.
   
3. In the bootloader menu, toggle the menu option to indicate that the download image should be stored
   on the SmartMedia.
   
4. Download the Stepldr/IPL/diskimage combination (the stepldr and IPL are built as part of the BSP and
   the disk image is generated post-buildrel, provided the "Image Update" build environment is selected).
   The image will be written to the SmartMedia after download.
   
5. Power-off the system and jumper pins 1 and 2 of J33.  This selects a NAND boot.

6. Power-on the Samsung board and the image stored on the SmartMedia should be booted.  To rerun steps
   1-4 above, remove the jumper on J33.

10/27/03
JDG
