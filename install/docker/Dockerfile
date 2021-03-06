FROM library/ubuntu:16.04
MAINTAINER "Martin Aumueller" <aumueller@hlrs.de>

ENV PAR -j8
ENV ARCHSUFFIX xerusopt
ENV BUILDTYPE Release
ENV PREFIX /usr
ENV BUILDDIR /build

RUN apt-get update -y && apt-get install -y \
        git bison flex swig cmake make file \
        libxerces-c-dev \
        qttools5-dev qtscript5-dev libqt5scripttools5 libqt5svg5-dev libqt5opengl5-dev libqt5webkit5-dev \
        libmpich-dev mpich \
        libopenscenegraph-dev libtiff-dev libpng-dev libgif-dev libjpeg-dev \
        libtbb-dev libglew-dev \
        libgdcm2-dev libnifti-dev libteem-dev libvolpack1-dev \
        libsnappy-dev zlib1g-dev \
        libvtk6-dev \
        inventor-dev freeglut3-dev libxmu-dev libxi-dev \
        python3-dev libpython3-dev \
        pyqt5-dev-tools python3-pyqt5 \
        libboost-atomic-dev libboost-chrono-dev libboost-date-time-dev libboost-exception-dev libboost-filesystem-dev \
        libboost-iostreams-dev libboost-locale-dev libboost-log-dev libboost-math-dev libboost-program-options-dev libboost-python-dev \
        libboost-random-dev libboost-regex-dev libboost-serialization-dev libboost-system-dev libboost-thread-dev libboost-timer-dev \
        libboost-tools-dev libboost-dev \
        libassimp-dev

WORKDIR ${BUILDDIR}
RUN git clone --recursive git://github.com/hlrs-vis/covise.git \
        && cd ${BUILDDIR}/covise \
        && export COVISEDIR=${BUILDDIR}/covise \
        && export COVISEDESTDIR=${COVISEDIR} \
        && export QT_SELECT=5 \
        && mkdir -p build.covise \
        && cd build.covise \
        && cmake .. -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_BUILD_TYPE=${BUILDTYPE} -DCOVISE_WARNING_IS_ERROR=FALSE -DCOVISE_USE_VISIONARAY=FALSE \
        && make ${PAR} install \
        && cd ${BUILDDIR}/covise \
        && mkdir -p build.cover \
        && cd build.cover \
        && cmake ../src/OpenCOVER -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_BUILD_TYPE=${BUILDTYPE} -DCOVISE_WARNING_IS_ERROR=FALSE \
        && make ${PAR} install \
        && cd ${BUILDDIR}/covise/Python \
        && make # install
WORKDIR /

#ENTRYPOINT ["/usr/bin/covise"]
CMD ["/usr/bin/covise"]
