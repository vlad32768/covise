SET(HEADERS
  OpenFoam.h
)

SET(SOURCES
  OpenFoam.cpp
)

ADD_COVISE_MODULE(Filter OpenFoam ${HEADERS} ${SOURCES})
TARGET_LINK_LIBRARIES(OpenFoam  coApi coAppl coCore )

COVISE_WNOERROR(OpenFoam)

COVISE_INSTALL_TARGET(OpenFoam)
