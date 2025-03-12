
echo "------------------------------------------------------------------"
echo "STATUS: sourcing ~/.bash_profile..."
echo "------------------------------------------------------------------"

# Note: this file should be sourced at bottom of from .bashrc
# using
#
# if [ -r ~/.bash_profile ]
# then
# 	. ~/.bash_profile
# fi
#

##############################################################################
# core bash functions
##############################################################################
_exportEnv() {
	local v_name=$1
	local v_value=$2
	echo "STATUS: setting "${v_name}"="${v_value}
	export ${v_name}"="${v_value}
}

_getIP() {
	local host=$1
	if [ `hostname`==${host} ]
	then
		local hostname=`hostname -I`
	else
		local hostname=`ssh ${host} hostname -I`
	fi
    local ip_arr=($hostname)
    local ip=${ip_arr[0]}
	printf ${ip}
}

##############################################################################
# env variables
##############################################################################

_exportEnv _HOST `hostname`
_exportEnv USER `whoami`

# set the time zone
_exportEnv TZ America/Los_Angeles

if [ $USER = "root" ]
then
	_exportEnv HOME "/root"
else
	_exportEnv HOME "/home/${USER}"
fi

# host and ip address
_exportEnv _HOST "/repo_dir"
_exportEnv _HOST_IP $(_getIP ${_HOST})


##############################################################################
# path adjustments
##############################################################################

echo "STATUS: adding to PATH..." 

_appendPath() {
	local dir=$1
	echo "STATUS: appending to PATH ${dir}"
	export PATH=$PATH:${dir}
}

# golang vars
_exportEnv GOPATH $HOME/go

# java
_exportEnv JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/

# golang
_exportEnv GOPATH $HOME/go
_appendPath /usr/local/go/bin

# rust
_appendPath /${HOME}/.cargo/bin

# append dirs to path
_appendPath ~/bin
_appendPath .

##############################################################################
# git settings
##############################################################################

echo "STATUS: configuring git settings..."

# git setup
git config --global user.name "Gabriel Solomon"
git config --global user.email gsolomon@scu.edu
git config --global push.default simple

git config --global push.default simple
git config --global core.editor "'/usr/bin/vi'"

echo "STATUS: adding gitl command..."
alias gitl="git log"

echo "STATUS: adding gitl1 command..."
alias gitl1="git log --oneline"

echo "STATUS: adding gitb command..."
alias gitb="git branch"

echo "STATUS: adding gitc command..."
alias gitc="git checkout"

##############################################################################
# colors
##############################################################################

echo "STATUS: adding _getColor function"...
_getColor() {
	# colors are from:
	# https://dev.to/ifenna__/adding-colors-to-bash-scripts-48g4

	local col=$1

	# foreground colors
	local fg_black=30
	local fg_red=31
	local fg_green=32 
	local fg_yellow=33
	local fg_blue=34 
	local fg_magenta=35
	local fg_cyan=36
	local fg_lightgray=37
	local fg_gray=90
	local fg_light_red=91
	local fg_light_green=92
	local fg_light_yellow=93
	local fg_light_blue=94
	local fg_light_magenta=95
	local fg_light_cyan=96
	local fg_white=97

	# background colors
	local bg_black=40
	local bg_red=41
	local bg_green=42 
	local bg_yellow=43
	local bg_blue=44 
	local bg_magenta=45
	local bg_cyan=46
	local bg_lightgray=47
	local bg_gray=100
	local bg_light_red=101
	local bg_light_green=102
	local bg_light_yellow=103
	local bg_light_blue=104
	local bg_light_magenta=105
	local bg_light_cyan=106
	local bg_white=107

	# style
	local normal=0
	local bold=1

	case $col in
	"start")
		local val="\e["
		;;
	"end")
		local val="\e[0m"
		;;
	*)
		# default, all colors
		local val="${bold};${!col}m"
		;;
	esac

	echo $val
}


##############################################################################
# color prompt
##############################################################################

# color test
# declare -a arr=("red" "blue")
# for col in "${arr[@]}"
# do
# 	printf "col="$col
# 	printf " "
# 	printf $(_getColor "start")$(_getColor $col)"HELLO WORLD\n"$(_getColor "end")
# done

# declaring some color variables
declare -A startColor=$(_getColor "start")
declare -A endColor=$(_getColor "end")

echo "STATUS: creating prompt..."
# creates a nice prompt
_createPrompt() {
	local startColor=$(_getColor "start")
	local endColor=$(_getColor "end")
	local hostname=`hostname`

	case $hostname in

	ubuntu*)
		local user="${startColor}$(_getColor "fg_light_red")\u${endColor}"
		local atSign="${startColor}$(_getColor "fg_gray")@${endColor}"
		local host="${startColor}$(_getColor "fg_light_red")\h${endColor}"
		local cwd="${startColor}$(_getColor "fg_light_red")\W${endColor}"
		;;
	*)
		# default prompt
		local user="${startColor}$(_getColor "fg_magenta")\u${endColor}"
		local atSign="${startColor}$(_getColor "fg_gray")@${endColor}"
		local host="${startColor}$(_getColor "fg_magenta")\h${endColor}"
		local cwd="${startColor}$(_getColor "fg_gray")\W${endColor}"
		;;
	esac

	# if root make user special red color
	if [ $USER = "root" ]
	then
		local user="${startColor}$(_getColor "bg_red")\u${endColor}"
	fi
	
	local prompt="[${user}${atSign}${host}:${cwd}]:"

	echo $prompt
}

PS1="$(_createPrompt)"

echo "STATUS: adding color to ls..."
alias ls='ls --color=auto'

##############################################################################
# aliases
##############################################################################

echo "STATUS: adding alias ll..."
alias ll='ls -alF'
echo "STATUS: adding alias la..."
alias la='ls -a'

echo "STATUS: adding alias h (history)..."
alias h='history'

# findit, that pipes errors to /dev/null
echo "STATUS: adding alias findit..."
_findit() {
	local arg=$1
	local cmd="find . -name ${arg} 2> /dev/null"
	echo STATUS: executing $cmd
	echo $cmd | sh
}
alias findit='_findit'

# cppre and mvpre commands
echo "STATUS: adding cppre command..."
function _cppre() {
	local from=$1
	local to=$2
	/usr/local/bin/copy_prefix.py $from $to --copy
}
alias cppre='_cppre'

echo "STATUS: amvpre command..."
alias mvpre="/usr/local/bin/copy_prefix.py ${1} --move"

# sorted env variables
echo "STATUS: changing env to sort environment variables..."
alias env='env | sort '

##############################################################################
# nvm setup
##############################################################################

# if nvm is not installed, install it and latest version of node
echo "STATUS: configuring nvm..."
if [ ! -d ~/.nvm ]
then
	echo "STATUS: installing nvm for user..."
	curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.5/install.sh | bash
fi
export NVM_DIR="$([ -z "${XDG_CONFIG_HOME-}" ] && printf %s "${HOME}/.nvm" || printf %s "${XDG_CONFIG_HOME}/nvm")"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh" # This loads nvm
# install the latest version of node
declare -A nodeVersion="21.1.0"
declare -A currVersion=`node -v`
if [ "v"$nodeVersion = $currVersion ]
then
	echo "STATUS: nvm ${nodeVersion} is already installed"
else
	echo "STATUS: installing nvm ${nodeVersion}..."
	nvm install $nodeVersion
	nvm use $nodeVersion
fi
# use the latest version of node
nvm use $nodeVersion >& /dev/null

echo "STATUS: cd to ${HOME}..."
cd $HOME

# if interactive, enable copy/paste to/from clipboard
if /usr/bin/autocutsel -s CLIPBOARD -fork > /dev/null 2>&1
then
	echo "STATUS: enabling copy/paste to terminal..."
fi


##############################################################################
# sshd
##############################################################################

# start ssh service, so we can ssh into from vscode, et cetera
# echo "STATUS: starting ssh service, if needed"
declare _ssh_is_running=`service ssh status | grep unning`
if [ -z "${_ssh_is_running}" ]
then
    echo "STATUS: starting ssh"
    service ssh start
else
	echo "STATUS: ssh is already running"
fi



##############################################################################
# avswu-veins set up
##############################################################################
echo "------------------------------------------------------------------"
echo "STATUS: avswu and veins specific setup"
echo "------------------------------------------------------------------"

# add aliases for building veins

echo "STATUS: adding simmake command..."
function _simmake() {
	# create compile_commands.json
	echo "--------------- creating compile_commands.json ---------------"
	pushd /home/avswu-veins/src
 	/home/.local/bin/compiledb make CXX=/usr/bin/clang++-17
	popd

	# run master make file for opp_makefile
	echo "--------------- compiling veins-client ---------------"
	pushd /home/avswu/veins-client
	make CXX=/usr/bin/clang++-17
	popd
	
	# run source make file
	echo "--------------- compiling avswu ---------------"
	pushd /home/avswu-veins
	make CXX=/usr/bin/clang++-17
	popd
}
alias simmake='_simmake'

echo "STATUS: adding simmakeverbose command..."
function _simmakeverbose() {
	# create compile_commands.json
	echo "--------------- creating compile_commands.json ---------------"
	pushd /home/avswu-veins/src
 	/home/.local/bin/compiledb make CXX=/usr/bin/clang++-17
	popd

	# run master make file for opp_makefile
	echo "--------------- compiling veins-client ---------------"
	pushd /home/avswu/veins-client
	make VERBOSE=1 CXX=/usr/bin/clang++-17
	popd
	
	# run source make file
	echo "--------------- compiling avswu ---------------"
	pushd /home/avswu-veins
	make V=1 CXX=/usr/bin/clang++-17
	popd
}
alias simmakeverbose='_simmakeverbose'

echo "STATUS: adding simquick command..."
function _simquick() {
	local argc=$#
    local argv=("$@")
    # printf "argc=$argc\n"
    # printf "argv=%s\n" "${argv[*]}"

	if [ $argc = 0 ]
	then
		local run_number=0
	else
		local run_number="${argv[0]}"
	fi

	local avswu_dir="/home/${USER}/avswu-veins"

	local command_args="..:../../src:../../../src/veins/examples/veins:../../../src/veins/src/veins -l ../../../src/veins/src/veins"
	local omnetpp_cmds="-r $run_number -u Cmdenv -c General omnetpp.ini --cmdenv-express-mode=true --cmdenv-log-level=off"
	local prog="${avswu_dir}/src/avswu"

	local cmd="${prog} -m -n ${command_args} ${omnetpp_cmds}"

	# run command
	pushd ${avswu_dir}/simulations/veins_avswu
 	printf "running cmd=${cmd}"
	${cmd}
	popd

}
alias simquick='_simquick'

# export to function sub shells
export -f _simquick

echo "STATUS: adding siminfolevel command..."
function _siminfolevel() {
	local avswu_dir="/home/${USER}/avswu-veins"

	local command_args="..:../../src:../../../src/veins/examples/veins:../../../src/veins/src/veins -l ../../../src/veins/src/veins"
	local omnetpp_cmds="-r 0 -u Cmdenv -c General omnetpp.ini --cmdenv-express-mode=true --cmdenv-log-level=info --record-eventlog=false"
	local prog="${avswu_dir}/src/avswu"

	local cmd="${prog} -m -n ${command_args} ${omnetpp_cmds}"

	# run command
	pushd ${avswu_dir}/simulations/veins_avswu
 	printf "running cmd=${cmd}"
	${cmd}
	popd

}
alias siminfolevel='_siminfolevel'

# export to function sub shells
export -f _siminfolevel

echo "STATUS: adding simdebuglevel command..."
function _simdebuglevel() {
	local avswu_dir="/home/${USER}/avswu-veins"

	local command_args="..:../../src:../../../src/veins/examples/veins:../../../src/veins/src/veins:../../../src/inet4/src -l ../../../src/veins/src/veins"
	local omnetpp_cmds="-r 0 -u Cmdenv -c General omnetpp.ini --cmdenv-express-mode=false --cmdenv-log-level=debug --record-eventlog=true"
	local prog="${avswu_dir}/src/avswu"

	local cmd="${prog} -m -n ${command_args} ${omnetpp_cmds}"

	# run command
	pushd ${avswu_dir}/simulations/veins_avswu
 	printf "running cmd=${cmd}"
	${cmd}
	popd

}
alias simdebuglevel='_simdebuglevel'

# export to function sub shells
export -f _simdebuglevel

echo "STATUS: adding simsaveresults command..."
function _simsaveresults() {
	local results_dir="/home/avswu-veins/simulations/veins_avswu/results"
	local save_dir="/home/avswu/saved-sim-results" 

	mkdir -p ${save_dir}

	local input_prefix="${results_dir}/General-#0"
	local suffix=`date +"%m_%d_%Y_%H_%M_%S"`
	local output_prefix="${save_dir}/sim_results_${suffix}"

	local startColor=$(_getColor "start")
	local endColor=$(_getColor "end")

	# convert to csv
	scavetool x ${input_prefix}.sca -o ${output_prefix}-scalar.csv
	scavetool x ${input_prefix}.vec -o ${output_prefix}-vector.csv

	printf "\n"
	local msg="saved ${output_prefix}-scalar.csv"
	local color_msg="${startColor}$(_getColor "bg_green")${msg}${endColor}"
	printf "${color_msg}\n"

	local msg="saved ${output_prefix}-vector.csv"
	local color_msg="${startColor}$(_getColor "bg_green")${msg}${endColor}"
	printf "${color_msg}\n"
	printf "\n"

}
alias simsaveresults='_simsaveresults'

alias simgrep="grep \"\[avswu\" /home/avswu-veins/simulations/veins_avswu/results/General-#0.elog"

# start avswu tools
echo "STATUS: adding simpolka command..."
alias simpolka="/home/avswu/avswu-node/start-polka.sh"
echo "STATUS: adding simserver command..."
alias simserver="cd /home/avswu/veins-server; npm start"
echo "STATUS: adding simsumo command..."
alias simsumo="/home/src/veins/bin/veins_launchd -vv -c /home/src/sumo-1.11.0/bin/sumo"


# source rust environment variables
echo "STATUS: configuring rust..."
. "$HOME/.cargo/env"

# openssl
echo "STATUS: configuring openssl..."
_exportEnv LD_LIBRARY_PATH "${LD_LIBRARY_PATH}:/usr/local/lib:/usr/local/gf-complete/src/.libs:/usr/local/openssl-3.3.0"

echo "STATUS: updating PATH for sumo, omnet++, veins..."

# sumo bin
_appendPath ~/src/sumo-1.11.0/bin

# veins bin
_appendPath ~/src/veins/bin

# omnetpp bin
_appendPath ~/src/omnetpp-5.7/bin

# aliases to start veins/sumo
alias sumostart="veins_launchd -vv -c sumo"

# openssl 3.3.0
_appendPath /usr/local/openssl-3.3.0/apps

# add go to path for ipfs
_appendPath /usr/local/go/bin

# set buffer size to accomodate ipfs file i/o
# echo "STATUS: configuring ipfs buffer..."
# sysctl -w net.core.rmem_max=7500000
# sysctl -w net.core.wmem_max=7500000

echo "------------------------------------------------------------------"
echo "STATUS: done"
echo "------------------------------------------------------------------"
