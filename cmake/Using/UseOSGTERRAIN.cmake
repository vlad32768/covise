MACRO(USE_OSGTERRAIN)
  COVISE_FIND_PACKAGE(osgTerrain)
  IF ((NOT OSGTERRAIN_FOUND)  AND (${ARGC} LESS 1))
    USING_MESSAGE("Skipping because of missing OSGTERRAIN")
    RETURN()
  ENDIF((NOT OSGTERRAIN_FOUND) AND (${ARGC} LESS 1))
  IF(NOT OSGTERRAIN_USED AND OSGTERRAIN_FOUND)
    SET(OSGTERRAIN_USED TRUE)
    USE_OPENGL()
    INCLUDE_DIRECTORIES(${OSGTERRAIN_INCLUDE_DIR})
    SET(EXTRA_LIBS ${EXTRA_LIBS} ${OSGTERRAIN_LIBRARIES})
  ENDIF()
ENDMACRO(USE_OSGTERRAIN)

