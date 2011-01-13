# Autobuilder for USB-K CryptoBridge

(automake --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have automake installed to compile Stardict";
    echo
    exit 1
}

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have autoconf installed to compile Stardict";
    echo
    exit 1
}

echo "Generating configuration files for USB-K, please wait..."
echo

echo "Running aclocal..."
aclocal || exit 1

# echo "Running autoheader..."
# autoheader || exit 1

echo "Running automake --add-missing --copy..."
automake --add-missing --copy;

echo "Running autoconf..."
autoconf || exit 1

echo "Running automake..."
automake || exit 1

