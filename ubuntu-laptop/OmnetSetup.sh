cd $HOME/src/omnetpp-5.7
source setenv
sed -i 's/WITH_OSGEARTH=yes/WITH_OSGEARTH=no/g' configure.user
sed -i 's/WITH_OSG=yes/WITH_OSG=no/g' configure.user
sed -i 's/WITH_QTENV=yes/WITH_QTENV=no/g' configure.user
sed -i 's/PREFER_QTENV=yes/PREFER_QTENV=no/g' configure.user
export PATH="$VEINS_INSTALL_DIR/$OMNETPP_VERSION/bin:$PATH"
./configure
make
make install
