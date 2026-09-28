CXX = zig c++
CFLAGS = -Wall -Wextra -ggdb -std=c++20 -O3
CPPFLAGS = -MMD -MP 

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

quasar: $(OBJS) 
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

-include $(DEPS)

%.o: %.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) $(DEPS) quasar
