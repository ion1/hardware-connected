# Usage example:
#
# BINARIES := main
#
# main_SRC := foo.c bar.c
# main_PKG := gtk+-2.0
#
# include generic.mk
#
# override CFLAGS += -std=gnu99

CFLAGS  = -W -Wall -g
LDFLAGS = -Wl,-O1

define pkgconfig_t
  $1_CFLAGS  := $$(shell pkg-config --cflags $1)
  $1_LDFLAGS := $$(shell pkg-config --libs   $1)

  CFLAGS += $$($1_CFLAGS)
endef

define binary_t
  ALL_SRC += $$($1_SRC)
  $1_OBJ  += $$($1_SRC:.c=.o)
  ALL_OBJ += $$($1_OBJ)

  $$(foreach pkg,$$($1_PKG),$$(eval $$(call pkgconfig_t,$$(pkg))))

  $1 : $$($1_OBJ)
	$$(CC) $$(LDFLAGS) $$(foreach pkg,$$($1_PKG),$$($$(pkg)_LDFLAGS)) \
	  -o$$@ $$^
endef

$(foreach binary,$(BINARIES),$(eval $(call binary_t,$(binary))))

ALL_DEP := $(ALL_OBJ:.o=.dep)

.PHONY: all
all : $(BINARIES)

.PHONY : clean
clean ::
	$(RM) $(ALL_OBJ) $(ALL_DEP) $(BINARIES)

%.dep : %.c
	$(CC) -o$@ -MM $<
-include $(ALL_DEP)
