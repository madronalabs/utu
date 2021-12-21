set(sources
  src/PartialReader.cpp
)

set(exe_sources
		cmd/src/main.cpp
		${sources}
)

set(headers
    include/utu/utu.h
    include/utu/Partial.h
    include/utu/PartialData.h
    include/utu/PartialReader.h
    src/SerializerImpl.h
)

set(test_sources
  src/test_json.cpp
)
