{
  "targets": [
    {
      "target_name": "gst-inspect",
      "sources": [ "src/GLibHelpers.cpp", "src/Inspect.cpp" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        '<!@(pkg-config gstreamer-1.0 --cflags-only-I | sed s/-I//g)',
      ],
      "cflags": [
        "-Wno-cast-function-type -Wno-unused-result"
      ],
      "libraries": [
        '<!@(pkg-config gstreamer-1.0 --libs)',
      ]
    },
    {
      "target_name": "gst-discover",
      "sources": [ "src/GLibHelpers.cpp", "src/Discover.cpp", "src/DiscoverInit.cpp" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        '<!@(pkg-config gstreamer-1.0 --cflags-only-I | sed s/-I//g)',
        '<!@(pkg-config gstreamer-base-1.0 --cflags-only-I | sed s/-I//g)',
        '<!@(pkg-config gstreamer-pbutils-1.0 --cflags-only-I | sed s/-I//g)',
      ],
      "cflags": [
        "-Wno-cast-function-type -Wno-unused-result"
      ],
      "libraries": [
        '<!@(pkg-config gstreamer-1.0 --libs)',
        '<!@(pkg-config gstreamer-base-1.0 --libs)',
        '<!@(pkg-config gstreamer-pbutils-1.0 --libs)',
      ]
    },
  ]
}