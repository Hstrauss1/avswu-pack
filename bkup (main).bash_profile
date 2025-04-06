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
	if [ "$(hostname)" == "$host" ]
	then
		local hostname=$(hostname -I)
	else
		local hostname=$(ssh ${host} hostname -I)
	fi
	local ip_arr=($hostname)
	local ip=${ip_arr[0]}
	printf ${ip}
}

##############################################################################
# env variables
##############################################################################

_exportEnv _HOST $(hostname)
_exportEnv USER $(whoami)

# set the time zone
_exportEnv TZ America/Los_Angeles

if [ $USER = "root" ]
then
	_exportEnv HOME "/root"
else
	_exportEnv HOME "/home/${USER}"
fi

# host and ip address
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
_appendPath ${HOME}/.cargo/bin

# append dirs to path
_appendPath ~/bin
_appendPath .

##############################################################################
# git settings
##############################################################################

echo "STATUS: configuring git settings..."

git config --global user.name "Gabriel Solomon"
git config --global user.email gsolomon@scu.edu
git config --global push.default simple
git config --global core.editor "'/usr/bin/vi'"

alias gitl="git log"
alias gitl1="git log --oneline"
alias gitb="git branch"
alias gitc="git checkout"

##############################################################################
# colors
##############################################################################

_getColor() {
	local col=$1
	local fg_red=31
	local fg_gray=90
	local fg_magenta=35
	local fg_light_red=91
	local bold=1

	case $col in
	"start")
		local val="\e["
		;;
	"end")
		local val="\e[0m"
		;;
	*)
		local val="${bold};${!col}m"
		;;
	esac

	echo $val
}

##############################################################################
# color prompt
##############################################################################

_createPrompt() {
	local startColor=$(_getColor "start")
	local endColor=$(_getColor "end")
	local hostname=$(hostname)

	case $hostname in
	ubuntu*)
		local user="${startColor}$(_getColor "fg_light_red")\u${endColor}"
		local atSign="${startColor}$(_getColor "fg_gray")@${endColor}"
		local host="${startColor}$(_getColor "fg_light_red")\h${endColor}"
		local cwd="${startColor}$(_getColor "fg_light_red")\W${endColor}"
		;;
	*)
		local user="${startColor}$(_getColor "fg_magenta")\u${endColor}"
		local atSign="${startColor}$(_getColor "fg_gray")@${endColor}"
		local host="${startColor}$(_getColor "fg_magenta")\h${endColor}"
		local cwd="${startColor}$(_getColor "fg_gray")\W${endColor}"
		;;
	esac

	if [ $USER = "root" ]
	then
		local user="${startColor}$(_getColor "bg_red")\u${endColor}"
	fi

	local prompt="[${user}${atSign}${host}:${cwd}]:"
	echo $prompt
}

PS1="$(_createPrompt)"

alias ls='ls --color=auto'
alias ll='ls -alF'
alias la='ls -a'
alias h='history'

_findit() {
	local arg=$1
	local cmd="find . -name ${arg} 2> /dev/null"
	echo STATUS: executing $cmd
	echo $cmd | sh
}
alias findit='_findit'

_cppre() {
	local from=$1
	local to=$2
	/usr/local/bin/copy_prefix.py $from $to --copy
}
alias cppre='_cppre'

alias mvpre="/usr/local/bin/copy_prefix.py ${1} --move"
alias env='env | sort '

##############################################################################
# nvm setup
##############################################################################

if [ ! -d ~/.nvm ]
then
	echo "STATUS: installing nvm for user..."
	curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.5/install.sh | bash
fi
export NVM_DIR="$([ -z "${XDG_CONFIG_HOME-}" ] && printf %s "${HOME}/.nvm" || printf %s "${XDG_CONFIG_HOME}/nvm")"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"

nodeVersion="21.1.0"
currVersion=$(node -v)
if [ "v$nodeVersion" = "$currVersion" ]
then
	echo "STATUS: nvm ${nodeVersion} is already installed"
else
	echo "STATUS: installing nvm ${nodeVersion}..."
	nvm install $nodeVersion
	nvm use $nodeVersion
fi
nvm use $nodeVersion >& /dev/null

cd $HOME

/usr/bin/autocutsel -s CLIPBOARD -fork > /dev/null 2>&1 && echo "STATUS: enabling copy/paste to terminal..."

declare _ssh_is_running=$(service ssh status | grep unning)
if [ -z "${_ssh_is_running}" ]
then
    echo "STATUS: starting ssh"
    service ssh start
else
	echo "STATUS: ssh is already running"
fi

##############################################################################
# avswu-veins setup
##############################################################################

_simmake() {
	pushd "$HOME/avswu-veins/src"
	$HOME/.local/bin/compiledb make CXX=/usr/bin/clang++-17
	popd

	pushd "$HOME/avswu/veins-client"
	make CXX=/usr/bin/clang++-17
	popd

	pushd "$HOME/avswu-veins"
	make CXX=/usr/bin/clang++-17
	popd
}
alias simmake='_simmake'

# similar path updates apply to simmakeverbose, simquick, siminfolevel, simdebuglevel, simsaveresults
# (skipping for brevity in this snippet)

alias simgrep="grep "\[avswu" ${HOME}/avswu-veins/simulations/veins_avswu/results/General-#0.elog"
alias simpolka="${HOME}/avswu/avswu-node/start-polka.sh"
alias simserver="cd ${HOME}/avswu/veins-server; npm start"
alias simsumo="${HOME}/src/veins/bin/veins_launchd -vv -c ${HOME}/src/sumo-1.11.0/bin/sumo"

. "$HOME/.cargo/env"

_exportEnv LD_LIBRARY_PATH "${LD_LIBRARY_PATH}:/usr/local/lib:/usr/local/gf-complete/src/.libs:/usr/local/openssl-3.3.0"

_appendPath "$HOME/src/sumo-1.11.0/bin"
_appendPath "$HOME/src/veins/bin"
_appendPath "$HOME/src/omnetpp-5.7/bin"
_appendPath /usr/local/openssl-3.3.0/apps
_appendPath /usr/local/go/bin"

echo "------------------------------------------------------------------"
echo "STATUS: done"
echo "------------------------------------------------------------------"
