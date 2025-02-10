
echo "STATUS:       setting env variables..."

_exportEnv() {
	local v_name=$1
	local v_value=$2
	echo "                   setting "${v_name}"="${v_value}
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

# global vars
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

# useful aliases
echo "------------------------------------------------------------------"
echo "STATUS:       ~/.bash_profile Adding useful aliases and functions..."
echo "------------------------------------------------------------------"

# aliases
echo "STATUS:       adding ll..."
alias ll='ls -alF'
echo "STATUS:       adding la..."
alias la='ls -a'

echo "STATUS:       adding h history alias..."
alias h='history'

# findit, that pipes errors to /dev/null
echo "STATUS:       adding findit..."
_findit() {
	local arg=$1
	local cmd="find . -name ${arg} 2> /dev/null"
	echo STATUS: executing $cmd
	echo $cmd | sh
}
alias findit='_findit'

echo "STATUS:       configuring git settings..."

# git setup
git config --global user.name "Gabriel Solomon"
git config --global user.email gsolomon@scu.edu
git config --global push.default simple

git config --global push.default simple
git config --global core.editor "'/usr/bin/vi'"

# make colors global
echo "STATUS:       setting up global color vars..."

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
echo "STATUS:       Adding _getColor function"...

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

echo "STATUS:       creating prompt..."
# creates a nice prompt
_createPrompt() {
	local startColor=$(_getColor "start")
	local endColor=$(_getColor "end")
	local hostname=`hostname`

	case $hostname in

	ubuntu*)
		local user="${startColor}$(_getColor "fg_yellow")\u${endColor}"
		local atSign="${startColor}$(_getColor "fg_gray")@${endColor}"
		local host="${startColor}$(_getColor "fg_yellow")\h${endColor}"
		local cwd="${startColor}$(_getColor "fg_yellow")\W${endColor}"
		;;
	*)
		# default prompt
		local user="${startColor}$(_getColor "fg_magenta")\u${endColor}"
		local atSign="${startColor}$(_getColor "fg_gray")@${endColor}"
		local host="${startColor}$(_getColor "fg_magenta")\h${endColor}"
		local cwd="${startColor}$(_getColor "fg_gray")\W${endColor}"
		;;
	esac

	local prompt="[${user}${atSign}${host}:${cwd}]:"

	echo $prompt
}

PS1="$(_createPrompt)"

echo "STATUS:       coloring ls..."
alias ls='ls --color=auto'

echo "STATUS:       adding to PATH..." 

_appendPath() {
	local dir=$1
	echo "                   appending ${dir}"
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

echo "STATUS:       Adding gitl command..."
alias gitl="git log"

echo "STATUS:       Adding gitl1 command..."
alias gitl1="git log --oneline"

echo "STATUS:       Adding gitb command..."
alias gitb="git branch"

echo "STATUS:       Adding gitc command..."
alias gitc="git checkout"

# cppre and mvpre commands
echo "STATUS:       Adding cppre command..."
function _cppre() {
	local from=$1
	local to=$2
	/usr/local/bin/copy_prefix.py $from $to --copy
}
alias cppre='_cppre'

echo "STATUS:       Adding mvpre command..."
alias mvpre="/usr/local/bin/copy_prefix.py ${1} --move"

# sorted env variables
echo "STATUS:       Changing env to sort environment variables..."
alias env='env | sort '

# if nvm is not installed, install it and latest version of node
echo "STATUS:       Configuring nvm..."
if [ ! -d ~/.nvm ]
then
	echo "STATUS:       Installing nvm for user..."
	curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.5/install.sh | bash
fi
export NVM_DIR="$([ -z "${XDG_CONFIG_HOME-}" ] && printf %s "${HOME}/.nvm" || printf %s "${XDG_CONFIG_HOME}/nvm")"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh" # This loads nvm
# install the latest version of node
declare -A nodeVersion="21.1.0"
declare -A currVersion=`node -v`
if [ "v"$nodeVersion = $currVersion ]
then
	echo "STATUS:       nvm ${nodeVersion} is already installed"
else
	echo "STATUS:       Installing nvm ${nodeVersion}..."
	nvm install $nodeVersion
	nvm use $nodeVersion
fi
# use the latest version of node
nvm use $nodeVersion

echo "STATUS:       cd to ${HOME}..."
cd $HOME

enable copy/paste to/from clipboard via vncserver
echo "STATUS:       Enable copy/paste to terminal..."
/usr/bin/autocutsel -s CLIPBOARD -fork

# start ssh service, so we can ssh into from vscode, et cetera
# echo "STATUS:       Starting ssh service, if needed"
declare _ssh_is_running=`service ssh status | grep unning`
if [ -z "${_ssh_is_running}" ]
then
    echo "STATUS:       starting ssh"
    service ssh start
else
	echo "STATUS:       ssh is already running"
fi

echo "------------------------------------------------------------------"
echo "STATUS: Done"
echo "------------------------------------------------------------------"
. "$HOME/.cargo/env"

_exportEnv LD_LIBRARY_PATH "${LD_LIBRARY_PATH}:/usr/local/lib:/usr/local/gf-complete/src/.libs"

