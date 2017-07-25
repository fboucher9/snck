# snck


Lightweight shell.


## Install


To install the `snck` application, you must compile from source and copy the
generated executable to a folder that is within `PATH` enumeration.


```
git clone http://github.com/fboucher9/snck
make
sudo cp snck /bin
echo '/bin/snck' | sudo tee -a /etc/shells
chsh -s /bin/snck
```

## Usage

```
snck [options]
```

*-c*

execute command

*-x*

trace

*-l*

execute login script

*-s*

read commands from standard input

*-i*

interactive

*-n*

dry run

## Builtins


*cd*



*set*

*unset*

*shell*


