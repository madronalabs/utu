set(lib_sources
  lib/src/PartialIO.cpp
)

set(exe_sources
    cmd/src/AudioPlayer.h
    cmd/src/AudioFile.cpp
    cmd/src/AudioFile.h
    cmd/src/Marshal.cpp
		cmd/src/main.cpp
		${lib_sources}
)

set(lib_headers
    lib/include/utu/utu.h
    lib/include/utu/Partial.h
    lib/include/utu/PartialData.h
    lib/include/utu/PartialIO.h
    lib/src/SerializerImpl.h
)

set(test_sources
  src/test_json.cpp
)
