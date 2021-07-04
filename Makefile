MAINDIR	= $(CURDIR)
SRC		= $(MAINDIR)/src
INC		= $(MAINDIR)/inc

CXXSRC	= $(wildcard $(SRC)/*.cpp) $(wildcard $(SRC)/*/*.cpp)
OBJ		= $(patsubst %.cpp,%.o,$(CXXSRC))
EXEC	= coap_server

CXXFLAGS	= -I$(INC) -Wall -Wextra -Werror -std=c++17 -g
LDFLAGS		= -lcoap-2-gnutls -lboost_program_options

all: $(OBJ)
	@echo -e "\033[33m  \033[1mLD\033[21m    \033[34m$(EXEC)\033[0m"
	@$(CXX) $(OBJ) $(LDFLAGS) -o $(EXEC)

clean:
	@echo -e "\033[33m  \033[1mCleaning $(EXEC)\033[0m"
	@rm -f $(OBJ) $(EXEC)

%.o: %.cpp
	@echo -e "\033[32m  \033[1mCC\033[21m    \033[34m$<\033[0m"
	@$(CXX) $(CXXFLAGS) -c -o $@ $<
