include $(TOPDIR)/config.mk

LIB	= lib$(SOC).a

START	=

OBJS	=  meminit.o
OBJS	+= hornet_serial.o
SOBJS	+= hornet_ddr_init.o
OBJS    += ag7240.o


all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS) $(OBJS-0)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
