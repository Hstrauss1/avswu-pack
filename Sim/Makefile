#
# master makefile for building veins avswu
#

VEINS_USER=gsolomon

# compile flags
AVSWU_CFLAGS = -KCFLAGS="-fPIC -O2"

INCLUDE_PATH += \
    -I/home/${VEINS_USER}/avswu/veins-client/veins_avswu \
    -I/usr/local/Jerasure/include \
    -I/usr/local/libipfs/bin

# -I/usr/local/openssl-3.3.0/include

# /usr/local/openssl-3.3.0/libssl.a \
# /usr/local/openssl-3.3.0/libcrypto.a \

# /usr/local/openssl-3.3.0/libcrypto.so.3 \
# /usr/local/openssl-3.3.0/libssl.so \

# /usr/local/lib/libssl.a \
# /usr/local/lib/libcrypto.a \


AVSWU_DEFINES = -D_HOSTNAME_INSTANT_VEINS

AVSWU_LIBS = \
    /home/${VEINS_USER}/avswu/veins-client/lib/libveins_avswu.a \
    /home/${VEINS_USER}/avswu/veins-client/lib/libveins_avswu_grpc_proto.a \
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
    /usr/local/libipfs/bin/libipfs-linux.a

EXTRA_OBJS = \
    ${AVSWU_LIBS} \
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
    -ldl \
    -lm \
    -lrt \
    -pthread \
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
    -lpthread \
    /usr/local/lib/libabsl_raw_logging_internal.a \
    /usr/local/lib/libabsl_log_severity.a \
    /usr/local/lib/libabsl_spinlock_wait.a \
    -lrt

VEINS_ARGS= \
    -KCXX=/usr/bin/clang++-17 \
	-KINET_PROJ=/home/${VEINS_USER}/src/inet4 \
	-KVEINS_PROJ=/home/${VEINS_USER}/src/veins \
	-DINET_IMPORT -DVEINS_IMPORT \
    ${AVSWU_DEFINES} \
    ${AVSWU_CFLAGS} \
	-I../grpc/grpc_veins_avswu \
	-I$$\(INET_PROJ\)/src \
	-I$$\(VEINS_PROJ\)/src \
	-I. -L$$\(INET_PROJ\)/src \
	-L$$\(VEINS_PROJ\)/src \
	-lINET$$\(D\) \
	-lveins$$\(D\)

all: checkmakefiles makefiles
	cd src && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile

makefiles:
	cd src && opp_makemake -f --deep $(VEINS_ARGS) $(INCLUDE_PATH) $(EXTRA_OBJS) -o avswu

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi
