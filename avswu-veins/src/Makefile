#
# OMNeT++/OMNEST Makefile for avswu
#
# This file was generated with the command:
#  opp_makemake -f --deep -KCXX=/usr/bin/clang++-17 -KINET_PROJ=/home/gsolomon/src/inet4 -KVEINS_PROJ=/home/gsolomon/src/veins -DINET_IMPORT -DVEINS_IMPORT -D_HOSTNAME_INSTANT_VEINS -KCFLAGS=-fPIC -O2 -I../grpc/grpc_veins_avswu -I$(INET_PROJ)/src -I$(VEINS_PROJ)/src -I. -L$(INET_PROJ)/src -L$(VEINS_PROJ)/src -lINET$(D) -lveins$(D) -I/home/gsolomon/avswu/veins-client/veins_avswu -I/usr/local/Jerasure/include -I/usr/local/libipfs/bin /home/gsolomon/avswu/veins-client/lib/libveins_avswu.a /home/gsolomon/avswu/veins-client/lib/libveins_avswu_grpc_proto.a /usr/local/lib/libabsl_flags.a /usr/local/lib/libabsl_flags_parse.a /usr/local/lib/libgrpc++_reflection.a /usr/local/lib/libgrpc++.a /usr/local/lib/libprotobuf.a /usr/local/lib/libgrpc.a /usr/local/lib/libcares.a /usr/local/lib/libz.a /usr/local/lib/libaddress_sorting.a /usr/local/lib/libgmp.a /usr/local/gf-complete/src/.libs/libgf_complete.a /usr/local/Jerasure/src/libtiming.a /usr/local/lib/libJerasure.so /usr/local/libipfs/bin/libipfs-linux.a /usr/local/lib/libre2.a /usr/local/lib/libupb_json_lib.a /usr/local/lib/libupb_textformat_lib.a /usr/local/lib/libupb_collections_lib.a /usr/local/lib/libupb.a /usr/local/lib/libutf8_range_lib.a /usr/local/lib/libz.a /usr/local/lib/libcares.a /usr/local/lib/libgpr.a /usr/local/lib/libabsl_random_distributions.a /usr/local/lib/libabsl_random_seed_sequences.a /usr/local/lib/libabsl_random_internal_pool_urbg.a /usr/local/lib/libabsl_random_internal_randen.a /usr/local/lib/libabsl_random_internal_randen_hwaes.a /usr/local/lib/libabsl_random_internal_randen_hwaes_impl.a /usr/local/lib/libabsl_random_internal_randen_slow.a /usr/local/lib/libabsl_random_internal_platform.a /usr/local/lib/libabsl_random_internal_seed_material.a /usr/local/lib/libabsl_random_seed_gen_exception.a /usr/local/lib/libssl.a /usr/local/lib/libcrypto.a -ldl -lm -lrt -pthread /usr/local/lib/libabsl_log_internal_check_op.a /usr/local/lib/libabsl_leak_check.a /usr/local/lib/libabsl_die_if_null.a /usr/local/lib/libabsl_log_internal_conditions.a /usr/local/lib/libabsl_log_internal_message.a /usr/local/lib/libabsl_log_internal_nullguard.a /usr/local/lib/libabsl_examine_stack.a /usr/local/lib/libabsl_log_internal_format.a /usr/local/lib/libabsl_log_internal_proto.a /usr/local/lib/libabsl_log_internal_log_sink_set.a /usr/local/lib/libabsl_log_sink.a /usr/local/lib/libabsl_log_entry.a /usr/local/lib/libabsl_log_initialize.a /usr/local/lib/libabsl_log_globals.a /usr/local/lib/libabsl_log_internal_globals.a /usr/local/lib/libabsl_statusor.a /usr/local/lib/libabsl_status.a /usr/local/lib/libabsl_strerror.a /usr/local/lib/libutf8_validity.a /usr/local/lib/libabsl_flags_usage.a /usr/local/lib/libabsl_flags_usage_internal.a /usr/local/lib/libabsl_flags.a /usr/local/lib/libabsl_flags_internal.a /usr/local/lib/libabsl_flags_marshalling.a /usr/local/lib/libabsl_flags_reflection.a /usr/local/lib/libabsl_flags_config.a /usr/local/lib/libabsl_flags_private_handle_accessor.a /usr/local/lib/libabsl_flags_commandlineflag.a /usr/local/lib/libabsl_flags_commandlineflag_internal.a /usr/local/lib/libabsl_flags_program_name.a /usr/local/lib/libabsl_cord.a /usr/local/lib/libabsl_cordz_info.a /usr/local/lib/libabsl_cord_internal.a /usr/local/lib/libabsl_cordz_functions.a /usr/local/lib/libabsl_cordz_handle.a /usr/local/lib/libabsl_crc_cord_state.a /usr/local/lib/libabsl_crc32c.a /usr/local/lib/libabsl_str_format_internal.a /usr/local/lib/libabsl_crc_internal.a /usr/local/lib/libabsl_crc_cpu_detect.a /usr/local/lib/libabsl_raw_hash_set.a /usr/local/lib/libabsl_hash.a /usr/local/lib/libabsl_bad_variant_access.a /usr/local/lib/libabsl_city.a /usr/local/lib/libabsl_low_level_hash.a /usr/local/lib/libabsl_bad_optional_access.a /usr/local/lib/libabsl_hashtablez_sampler.a /usr/local/lib/libabsl_exponential_biased.a /usr/local/lib/libabsl_synchronization.a /usr/local/lib/libabsl_graphcycles_internal.a /usr/local/lib/libabsl_kernel_timeout_internal.a /usr/local/lib/libabsl_time.a /usr/local/lib/libabsl_civil_time.a /usr/local/lib/libabsl_time_zone.a /usr/local/lib/libabsl_stacktrace.a /usr/local/lib/libabsl_symbolize.a /usr/local/lib/libabsl_strings.a /usr/local/lib/libabsl_string_view.a /usr/local/lib/libabsl_strings_internal.a /usr/local/lib/libabsl_int128.a /usr/local/lib/libabsl_throw_delegate.a /usr/local/lib/libabsl_malloc_internal.a /usr/local/lib/libabsl_debugging_internal.a /usr/local/lib/libabsl_demangle_internal.a /usr/local/lib/libabsl_base.a -lpthread /usr/local/lib/libabsl_raw_logging_internal.a /usr/local/lib/libabsl_log_severity.a /usr/local/lib/libabsl_spinlock_wait.a -lrt -o avswu
#

# Name of target to be created (-o option)
TARGET_DIR = .
TARGET_NAME = avswu$(D)
TARGET = $(TARGET_NAME)$(EXE_SUFFIX)
TARGET_IMPLIB = $(TARGET_NAME)$(IMPLIB_SUFFIX)
TARGET_IMPDEF = $(TARGET_NAME)$(IMPDEF_SUFFIX)
TARGET_FILES = $(TARGET_DIR)/$(TARGET)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(QTENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)
#USERIF_LIBS = $(QTENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I../grpc/grpc_veins_avswu \
    -I$(INET_PROJ)/src \
    -I$(VEINS_PROJ)/src \
    -I. \
    -I/home/gsolomon/avswu/veins-client/veins_avswu \
    -I/usr/local/Jerasure/include \
    -I/usr/local/libipfs/bin

# Additional object and library files to link with
EXTRA_OBJS = \
    /home/gsolomon/avswu/veins-client/lib/libveins_avswu.a \
    /home/gsolomon/avswu/veins-client/lib/libveins_avswu_grpc_proto.a \
    /usr/local/lib/libabsl_flags.a \
    /usr/local/lib/libabsl_flags_parse.a \
    /usr/local/lib/libgrpc++_reflection.a \
    /usr/local/lib/libgrpc++.a \
    /usr/local/lib/libprotobuf.a \
    /usr/local/lib/libgrpc.a \
    /usr/local/lib/libcares.a \
    /usr/local/lib/libz.a \
    /usr/local/lib/libaddress_sorting.a \
    /usr/local/lib/libgmp.a \
    /usr/local/gf-complete/src/.libs/libgf_complete.a \
    /usr/local/Jerasure/src/libtiming.a \
    /usr/local/lib/libJerasure.so \
    /usr/local/libipfs/bin/libipfs-linux.a \
    /usr/local/lib/libre2.a \
    /usr/local/lib/libupb_json_lib.a \
    /usr/local/lib/libupb_textformat_lib.a \
    /usr/local/lib/libupb_collections_lib.a \
    /usr/local/lib/libupb.a \
    /usr/local/lib/libutf8_range_lib.a \
    /usr/local/lib/libz.a \
    /usr/local/lib/libcares.a \
    /usr/local/lib/libgpr.a \
    /usr/local/lib/libabsl_random_distributions.a \
    /usr/local/lib/libabsl_random_seed_sequences.a \
    /usr/local/lib/libabsl_random_internal_pool_urbg.a \
    /usr/local/lib/libabsl_random_internal_randen.a \
    /usr/local/lib/libabsl_random_internal_randen_hwaes.a \
    /usr/local/lib/libabsl_random_internal_randen_hwaes_impl.a \
    /usr/local/lib/libabsl_random_internal_randen_slow.a \
    /usr/local/lib/libabsl_random_internal_platform.a \
    /usr/local/lib/libabsl_random_internal_seed_material.a \
    /usr/local/lib/libabsl_random_seed_gen_exception.a \
    /usr/local/lib/libssl.a \
    /usr/local/lib/libcrypto.a \
    /usr/local/lib/libabsl_log_internal_check_op.a \
    /usr/local/lib/libabsl_leak_check.a \
    /usr/local/lib/libabsl_die_if_null.a \
    /usr/local/lib/libabsl_log_internal_conditions.a \
    /usr/local/lib/libabsl_log_internal_message.a \
    /usr/local/lib/libabsl_log_internal_nullguard.a \
    /usr/local/lib/libabsl_examine_stack.a \
    /usr/local/lib/libabsl_log_internal_format.a \
    /usr/local/lib/libabsl_log_internal_proto.a \
    /usr/local/lib/libabsl_log_internal_log_sink_set.a \
    /usr/local/lib/libabsl_log_sink.a \
    /usr/local/lib/libabsl_log_entry.a \
    /usr/local/lib/libabsl_log_initialize.a \
    /usr/local/lib/libabsl_log_globals.a \
    /usr/local/lib/libabsl_log_internal_globals.a \
    /usr/local/lib/libabsl_statusor.a \
    /usr/local/lib/libabsl_status.a \
    /usr/local/lib/libabsl_strerror.a \
    /usr/local/lib/libutf8_validity.a \
    /usr/local/lib/libabsl_flags_usage.a \
    /usr/local/lib/libabsl_flags_usage_internal.a \
    /usr/local/lib/libabsl_flags.a \
    /usr/local/lib/libabsl_flags_internal.a \
    /usr/local/lib/libabsl_flags_marshalling.a \
    /usr/local/lib/libabsl_flags_reflection.a \
    /usr/local/lib/libabsl_flags_config.a \
    /usr/local/lib/libabsl_flags_private_handle_accessor.a \
    /usr/local/lib/libabsl_flags_commandlineflag.a \
    /usr/local/lib/libabsl_flags_commandlineflag_internal.a \
    /usr/local/lib/libabsl_flags_program_name.a \
    /usr/local/lib/libabsl_cord.a \
    /usr/local/lib/libabsl_cordz_info.a \
    /usr/local/lib/libabsl_cord_internal.a \
    /usr/local/lib/libabsl_cordz_functions.a \
    /usr/local/lib/libabsl_cordz_handle.a \
    /usr/local/lib/libabsl_crc_cord_state.a \
    /usr/local/lib/libabsl_crc32c.a \
    /usr/local/lib/libabsl_str_format_internal.a \
    /usr/local/lib/libabsl_crc_internal.a \
    /usr/local/lib/libabsl_crc_cpu_detect.a \
    /usr/local/lib/libabsl_raw_hash_set.a \
    /usr/local/lib/libabsl_hash.a \
    /usr/local/lib/libabsl_bad_variant_access.a \
    /usr/local/lib/libabsl_city.a \
    /usr/local/lib/libabsl_low_level_hash.a \
    /usr/local/lib/libabsl_bad_optional_access.a \
    /usr/local/lib/libabsl_hashtablez_sampler.a \
    /usr/local/lib/libabsl_exponential_biased.a \
    /usr/local/lib/libabsl_synchronization.a \
    /usr/local/lib/libabsl_graphcycles_internal.a \
    /usr/local/lib/libabsl_kernel_timeout_internal.a \
    /usr/local/lib/libabsl_time.a \
    /usr/local/lib/libabsl_civil_time.a \
    /usr/local/lib/libabsl_time_zone.a \
    /usr/local/lib/libabsl_stacktrace.a \
    /usr/local/lib/libabsl_symbolize.a \
    /usr/local/lib/libabsl_strings.a \
    /usr/local/lib/libabsl_string_view.a \
    /usr/local/lib/libabsl_strings_internal.a \
    /usr/local/lib/libabsl_int128.a \
    /usr/local/lib/libabsl_throw_delegate.a \
    /usr/local/lib/libabsl_malloc_internal.a \
    /usr/local/lib/libabsl_debugging_internal.a \
    /usr/local/lib/libabsl_demangle_internal.a \
    /usr/local/lib/libabsl_base.a \
    /usr/local/lib/libabsl_raw_logging_internal.a \
    /usr/local/lib/libabsl_log_severity.a \
    /usr/local/lib/libabsl_spinlock_wait.a

# Additional libraries (-L, -l options)
LIBS = $(LDFLAG_LIBPATH)$(INET_PROJ)/src $(LDFLAG_LIBPATH)$(VEINS_PROJ)/src  -lINET$(D) -lveins$(D) -ldl -lm -lrt -lpthread -lrt

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = \
    $O/veins_avswu/AvswuClientAV11p.o \
    $O/veins_avswu/AvswuDeliveryAV11p.o \
    $O/veins_avswu/AvswuManufacturer11p.o \
    $O/veins_avswu/AvswuNode11p.o \
    $O/veins_avswu/AvswuRSU11p.o \
    $O/veins_avswu/AvswuVeinsUtils.o \
    $O/veins_avswu/AvswuAV11pMessage_m.o

# Message files
MSGFILES = \
    veins_avswu/AvswuAV11pMessage.msg

# SM files
SMFILES =

# Other makefile variables (-K)
CXX=/usr/bin/clang++-17
INET_PROJ=/home/gsolomon/src/inet4
VEINS_PROJ=/home/gsolomon/src/veins
CFLAGS=-fPIC -O2

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
CONFIGFILE = $(shell opp_configfilepath)
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIBS = $(OPPMAIN_LIB) $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)
ifneq ($(PLATFORM),win32.x86_64)
LIBS += -Wl,-rpath,$(abspath $(INET_PROJ)/src) -Wl,-rpath,$(abspath $(VEINS_PROJ)/src)
endif

COPTS = $(CFLAGS) $(IMPORT_DEFINES) -DINET_IMPORT -DVEINS_IMPORT -D_HOSTNAME_INSTANT_VEINS $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH) -Pthread_API
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE (if COPTS has changed since last build)
# and make the object files depend on it
COPTS_FILE = $O/.last-copts
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
  $(shell $(MKPATH) "$O")
  $(file >$(COPTS_FILE),$(COPTS))
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
-include makefrag

#------------------------------------------------------------------------------

# Main target
all: $(TARGET_FILES)

$(TARGET_DIR)/% :: $O/%
	@mkdir -p $(TARGET_DIR)
	$(Q)$(LN) $< $@
ifeq ($(TOOLCHAIN_NAME),clang-msabi)
	-$(Q)-$(LN) $(<:%.dll=%.lib) $(@:%.dll=%.lib) 2>/dev/null
endif

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile $(CONFIGFILE)
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET) $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders smheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE) | msgheaders smheaders
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc -MD -MP -MF $O/$(basename $<)_m.h.d $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning $(TARGET)
	$(Q)-rm -rf $O
	$(Q)-rm -f $(TARGET_FILES)
	$(Q)-rm -f $(call opp_rwildcard, . , *_m.cc *_m.h *_sm.cc *_sm.h)

cleanall:
	$(Q)$(CLEANALL_COMMAND)
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

help:
	@echo "$$HELP_SYNOPSYS"
	@echo "$$HELP_TARGETS"
	@echo "$$HELP_VARIABLES"
	@echo "$$HELP_EXAMPLES"

# include all dependencies
-include $(OBJS:%=%.d) $(MSGFILES:%.msg=$O/%_m.h.d)
