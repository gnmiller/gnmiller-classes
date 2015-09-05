#name some definitions of colors...
clear='\e[0m'
black='\e[0;90m'
red='\e[0;91m'
bred='\e[1;91m'
green='\e[0;92m'
yellow='\e[0;93m'
byellow='\e[1;93m'
blue='\e[0;94m'
purple='\e[0;95m'
cyan='\e[0;96m'
white='\e[0;97m'

userhost="\[$green\]\u\[$clear\]@\[$cyan\]\H\[$clear\]"
curtime="\[$purple\]\A\[$clear\]"
workdir="\[$red\]\w\[$clear\]"

# set color and $/# based on EUID
if [ $EUID == 0 ]; then
	prompt="\[$bred\]"
else 
	prompt="\[$byellow\]"
fi
prompt=$prompt'\$'
prompt="$prompt\[$clear\]"

export PS1="[$userhost][$curtime][$workdir]\n$prompt "
