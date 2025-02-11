export SUMO_URL="https://sourceforge.net/projects/sumo/files/sumo/version%201.11.0/sumo-1.11.0.orig.tar.gz"
export SUMO_VERSION="sumo-1.11.0"


echo "Downloading SUMO..."
curl -L "https://sourceforge.net/projects/sumo/files/sumo/version%201.11.0/sumo_1.11.0.orig.tar.gz/download" -o "sumo-1.11.0.tar.gz"

# Wait 5 seconds to ensure the download is fully completed
sleep 7
tar -xzf "sumo-1.11.0.tar.gz"
rm "sumo-1.11.0.tar.gz"
mv "sumo-1.11.0" "$HOME/src"

