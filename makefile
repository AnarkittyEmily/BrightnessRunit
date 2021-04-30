install:
	@g++ src/main.cpp -o brightness_runit -std=c++20 -Wextra -Wall
	@mv brightness_runit /usr/bin

uninstall:
	@rm /usr/bin/brightness_runit
