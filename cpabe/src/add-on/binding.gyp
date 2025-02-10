{
    "targets": [
        {
            "target_name": "cpabeAddOn",
            "sources": ["cpabeAddOn.cpp",
                        "nodeAddOnUtils.cpp",
                        "cpabeFuncs.cpp",
                        ],
            "cflags": [
                "-Wno-cast-function-type",
                "-lglib-2.0",
                "-lgmp",
                "-lpbc",
                "-lcrypto",
                "-lbswabe",
                "-fPIC",
            ],
            "include_dirs":[
                "<!(node -e \"require('nan')\")",
                "/usr/include/glib-2.0",
                "/usr/lib/x86_64-linux-gnu/glib-2.0/include",
                "/usr/local/pbc-0.5.14/include",
                "/avswu/cpabe/cpabe-0.11"
            ],
            "libraries": [
                "/usr/local/lib/libbswabe.a",
                "/usr/local/lib/libpbc.so",
                "/usr/local/lib/libgmp.so",
                "/usr/lib/x86_64-linux-gnu/libglib-2.0.so",
            ],
        }
    ]
}
