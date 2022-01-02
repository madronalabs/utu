set(sources
  src/PartialIO.cpp
)

set(exe_sources
    cmd/src/AudioPlayer.h
    cmd/src/AudioFile.cpp
    cmd/src/AudioFile.h
    cmd/src/Marshal.cpp
		cmd/src/main.cpp
		${sources}
)

set(headers
    include/utu/utu.h
    include/utu/Partial.h
    include/utu/PartialData.h
    include/utu/PartialIO.h
    src/SerializerImpl.h
)

set(test_sources
  src/test_json.cpp
)
