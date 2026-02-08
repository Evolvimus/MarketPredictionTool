CXX = g++
CXXFLAGS = -std=c++17 -O3 -I/usr/local/include -I./include
LDFLAGS = -L/usr/local/lib -lcurl

TARGET = predict_server
OBJS = analysis.o analysis_storage.o market_data.o news_fetcher.o ollama_client.o server.o settings_storage.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
