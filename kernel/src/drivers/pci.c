/*
	Copyright 2012-2013 universe coding group (UCG) all rights reserved
	This file is part of the Universe Kernel.

	Universe Kernel is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	any later version.

	Universe Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Universe Kernel.  If not, see <http://www.gnu.org/licenses/>.
*/

/**

  @author Simon Diepold aka. Tdotu (Universe Team) <simon.diepold@infinitycoding.de>

 */

#define _PCI_C_

#include <drivers/pci.h>
#include <printf.h>
#include <heap.h>

uint32_t pci_read(uint8_t bus,uint8_t dev,uint8_t func,uint8_t offset)
{
    outl(PCI_CONFIG_ADDRESS,0x80000000 | (bus << 16) | (dev << 11) |( func << 8) | (offset & 0xFC));
    return inl(PCI_CONFIG_DATA) >> (8 * (offset % 4));
}



/**
 * reads a byte from the PCI configuration space
 * @param Bus
 * @param Device
 * @param function
 * @param Offset
 * @return value from the input adress
 */
uint8_t pci_readb(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset)
{
    return pci_read(bus, dev, func, offset) & 0xff;
}

/**
 * reads a word from the PCI configuration space
 * @param Bus
 * @param Device
 * @param function
 * @param Offset
 * @return value from the input adress
 */
inline uint16_t pci_readw(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset)
{
    return pci_read(bus, dev, func, offset) & 0xffff;
}


/**
 * reads a double word from the PCI configuration space
 * @param Bus
 * @param Device
 * @param function
 * @param Offset
 * @return value from the input adress
 */
inline uint32_t pci_readl(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset)
{
    return pci_read(bus, dev, func, offset);
}


/**
 * writes value into to PCI configuration space
 * @param Bus
 * @param Device
 * @param function
 * @param Offset
 * @param Value
 */
inline void pci_writeb(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint8_t value)
{
    outl(PCI_CONFIG_ADDRESS,0x80000000 | (bus << 16) | (dev << 11 ) | (func << 8) | (offset & 0xFC));
    outb(PCI_CONFIG_DATA + (offset & 3), value);
}


/**
 * writes value into to PCI configuration space
 * @param Bus
 * @param Device
 * @param function
 * @param Offset
 * @param Value
 */
inline void pci_writew(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint16_t value)
{
    outl(PCI_CONFIG_ADDRESS, 0x80000000 | (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC));
    outw(PCI_CONFIG_DATA + (offset & 2), value);
}


/**
 * writes value into to PCI configuration space
 * @param Bus
 * @param Device
 * @param function
 * @param Offset
 * @param Value
 */
inline void pci_writel(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value)
{
    outl(PCI_CONFIG_ADDRESS, 0x80000000 | (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC));
    outl(PCI_CONFIG_DATA, value);
}


/**
 * checks if PCI device exists
 *  @param Bus
 *  @param Device/Slot
 *  @return true (device exists) or false (device does not exist)
 */
bool pci_dev_exist(uint8_t bus, uint8_t dev, uint8_t func)
{
    uint16_t vendor_ID = pci_readw(bus, dev, func, 0);
    if(vendor_ID == 0 || vendor_ID == 0xFFFF)
        return false;
    return true;
}

char *pci_dev_names[][16]={
    {"non vga device", "vga device"}, {"SCSI", "IDE", "Floppy", "IPI", "RAID", "ATA-controler", "SATA-controler","SAS"},
    {"Ethernet", "Token Ring", "FDDI", "ATM", "ISDN", "FIP", "PICMG 2.14"}, {"VGA", "XGA", "3D"}, {"Video", "Audio", "Phone", "HD-Audio"},
    {"RAM", "Flash"},{"Host", "ISA", "EISA-Bridge", "MicroChannel", "PCI-Bridge", "PCMCIA-Bridge", "NuBus-Bridge", "CardBus-Bridge", "Raceway", "Semitransparente PCI/PCI-Bridge","InfinityBand"}
};


//Linked list of PCI devices
list_t *pci_dev_list;

#define PRINT_DEV_LIST


void INIT_PCI()
{
    #ifdef PRINT_DEV_LIST
        printf("PCI-devices:\n");
    #endif
    pci_dev_list = list_create();
    int dev,bus,func;

    for(bus = 0; bus < 8; bus++)
    {

        for(dev = 0; dev < 32; dev++)
        {

            for(func = 0; func < 8; func ++)
            {

                if(pci_dev_exist(bus, dev, func))
                {
                    bool multifunc = (pci_readb(bus, dev, func,PCI_HEADERTYPE) & 0x80) >> 7;

                    if(func && ! multifunc)
                        continue;

                    struct pci_dev *current_dev = malloc(sizeof(struct pci_dev));

                    current_dev->bus = bus;
                    current_dev->dev = dev;
                    current_dev->func = func;

                    uint32_t classcode = pci_readl(bus, dev, 0, PCI_REVISION);

                    current_dev->reversion_ID = (uint8_t)classcode;
                    current_dev->programming_interface = (uint8_t) (classcode >> 8);
                    current_dev->sub_class = (uint8_t) (classcode >> 16);
                    current_dev->base_class = (uint8_t) (classcode >> 24);

                    current_dev->device_ID = pci_readw(bus, dev, func, PCI_DEVICE_ID);
                    current_dev->vendor_ID = pci_readw(bus, dev, func, PCI_VENDOR_ID);
                    current_dev->header_type = pci_readb(bus, dev ,0, PCI_HEADERTYPE) ^ 0x80;
                    current_dev->multifunc = multifunc;

                    uint32_t irq_info = pci_readl(bus, dev, func, PCI_INTERRUPT);
                    current_dev->irq_num = (uint8_t) irq_info;
                    current_dev->irq_pin = (uint8_t) (irq_info >> 8);



                    // Standart Device
                    if(! (current_dev->header_type & 0xFF) )
                    {
                        int base;

                        for(base = 0; base < 6; base ++)
                        {
                            uint32_t current_base = pci_readl(bus, dev, func, PCI_BASE + (base * 4));

                            // get type
                            current_dev->base_adress[base].type = current_base & 1;

                            // save current adress
                            current_dev->base_adress[base].adress = current_base ^ 1;

                            // get reserved bits
                            pci_writel(bus, dev, func, PCI_BASE + (base * 4), 0xFFFFFFFF);
                            uint32_t temp_base = pci_readl(bus, dev, func, PCI_BASE + (base * 4));
                            temp_base = (~temp_base) | 1;
                            current_dev->base_adress[base].resb = 0;

                            int i;
                            for(i = 0; i < 32; i++)
                            {
                                if(temp_base & (1 << i) == 0)
                                    current_dev->base_adress[base].resb++;
                                else
                                    break;
                            }

                            //reset old state
                             pci_writel(bus, dev, func, PCI_BASE + (base * 4), current_base);
                        }


                    }
                    // Bridge Device
                    else
                    {
                        //TODO: Write Cases for Bridges
                    }

                    #ifdef PRINT_DEV_LIST
                        printf("device ID: %04X  vendor ID: %04X  bus: %d  port: %d  function: %d\n",current_dev->device_ID, current_dev->vendor_ID, current_dev->bus, current_dev->dev, current_dev->func);
                    #endif

                    list_push_front(pci_dev_list, current_dev);
                }
            }
        }
    }

}
