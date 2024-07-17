#include <Arduino.h>

#include "decaproto/encoder.h"
#include "decaproto/stream/stl.h"
#include "decaproto/stream/string_stream.h"
#include "proto/example.pb.h"

void fill_state(State *state, int i) {
  state->set_timestamp(i * 100);
  state->set_id(i);
  state->set_double_value(12.34);
  state->set_bool_value((i % 2) == 0);

  state->mutable_detail()->set_value_a(0.5);
  state->mutable_detail()->set_value_b(50.0);
}

Response create_response() {
  Response response;

  for (int i = 0; i < 8; i++) {
    fill_state(response.add_states(), i);
  }

  return response;
}

void test_encode_res() {
  Response response = create_response();

  std::string buffer;
  buffer.reserve(256);
  decaproto::StringOutputStream sos(&buffer);
  size_t written_size;

  uint32_t start = time_us_32();
  response.Encode(sos, written_size);
  uint32_t end = time_us_32();

  // Encoding should be finished within 1ms
  Serial.printf("Encoding Response took: %ld us\n", end - start);
  Serial.printf("Encoded size: %u bytes\n", written_size);
}

void test_encode_state() {
  State state;
  fill_state(&state, 10);

  std::string buffer;
  buffer.reserve(256);
  decaproto::StringOutputStream sos(&buffer);
  size_t written_size;

  uint32_t start = time_us_32();
  // EncodeMessage(sos, state, written_size);
  state.Encode(sos, written_size);
  uint32_t end = time_us_32();

  // Encoding should be finished within 1ms
  Serial.printf("Encoding State took: %ld us\n", end - start);
  Serial.printf("Encoded size: %u bytes\n", written_size);
}

void runTests() {
  test_encode_res();
  test_encode_state();
}

void setup() {
  // Wait for the host PC to be connected
  while (!Serial) {
    analogWrite(D25, 0);
    delay(100);
    analogWrite(D25, 100);
    delay(100);
  }

  runTests();
}

void loop() {
  analogWrite(D25, 0);
  delay(1000);
  analogWrite(D25, 100);
  delay(1000);
}
