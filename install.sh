
# Organize the FD

echo "Run install.sh script"

if -f bin; then

# Create directories
    echo "Create directories"
    mkdir bin
    mkdir usr ; mkdir usr/bin
    mkdir usr/icons

# Copy the files
    mv cat bin/cat
    cp cp bin/cp
    mv echo bin/echo
    mv mkdir bin/mkdir
    cp zsh bin/zsh
    mv ls bin/ls
    mv grep bin/grep
    mv ps bin/ps
fi
