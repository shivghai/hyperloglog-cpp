CPPFLAGS= -std=c++20

.PHONY:hyperloglog

hyperloglog:
	g++ $(CPPFLAGS) \
	main.cpp -o hyperloglog

hlib:
	emcc -lembind -o hll.js hyperloglog_bindings.cpp

wasm-test:
	emcc main.cpp

clean:
	rm hyperloglog