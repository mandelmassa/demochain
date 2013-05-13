SILENT	?= @
VERSION	 = 0.3

LIBDEMO	 = ../libdemo

CFLAGS	 =
CFLAGS	+= -I$(LIBDEMO)/inc

ifeq ($(OSTYPE),cygwin)
CFLAGS	+= -m32
CFLAGS	+= -mno-cygwin
endif

LDFLAGS	 =
LDFLAGS	+= -L$(LIBDEMO)
LDFLAGS	+= -ldemo

ifeq ($(OSTYPE),cygwin)
LDFLAGS	+= -mno-cygwin
endif

ifeq ($(DEBUG),YES)
CFLAGS	+= -g
else
CFLAGS	+= -O3
endif

ifeq ($(OSTYPE),cygwin)
BINARY	 = demochain.exe
else
BINARY	 = demochain
endif

OBJ	 = demochain.o

OBJS	 = $(addprefix $(OBJDIR)/,$(OBJ))

OBJDIR	 = obj
SRCDIR	 = src

default: all

#
# build targets
#

.PHONY: all clean

all: $(BINARY)

$(BINARY): $(OBJS)
	@echo "Linking $(OBJS) => $@"
	$(SILENT)gcc $(OBJS) $(LDFLAGS) -o $@

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	$(SILENT)mkdir $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "Compiling $< => $@"
	$(SILENT)gcc -c $(CFLAGS) $< -o $@

clean:
	$(SILENT)rm -fr $(OBJDIR) $(BINARY)
