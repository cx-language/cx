// JSON parse and walk, matching jsonparse.cx. core:encoding/json's Value
// union keeps integers and fractions apart when parse_integers is set.
package jsonparse

import "core:encoding/json"
import "core:fmt"
import "core:strconv"
import "core:strings"

checksum :: proc(value: json.Value) -> i64 {
	switch v in value {
	case json.String:
		return i64(len(v))
	case json.Integer:
		return i64(v)
	case json.Float:
		return i64(f64(v) * 1000)
	case json.Boolean:
		if bool(v) do return 1
		return 0
	case json.Null:
		return 0
	case json.Array:
		sum: i64
		for item in v {
			sum += checksum(item)
		}
		return sum
	case json.Object:
		sum: i64
		for key, item in v {
			sum += i64(len(key)) + checksum(item)
		}
		return sum
	}
	return 0
}

make_document :: proc() -> strings.Builder {
	doc := strings.builder_make()
	strings.write_string(&doc, "{\"users\":[")
	for i in 0 ..< 5000 {
		if i > 0 {
			strings.write_byte(&doc, ',')
		}
		strings.write_string(&doc, "{\"id\":")
		strings.write_int(&doc, i)
		strings.write_string(&doc, ",\"name\":\"user")
		strings.write_int(&doc, i)
		strings.write_string(&doc, "\",\"score\":")
		score := f32(i) * 1.5
		buf: [64]byte
		// strconv prefixes a '+'; JSON numbers do not.
		score_s := strconv.write_float(buf[:], f64(score), 'g', -1, 32)
		if len(score_s) > 0 && score_s[0] == '+' {
			score_s = score_s[1:]
		}
		strings.write_string(&doc, score_s)
		strings.write_string(&doc, ",\"active\":")
		strings.write_string(&doc, "true" if i % 2 == 0 else "false")
		strings.write_string(&doc, "}")
	}
	strings.write_string(&doc, "],\"meta\":{\"version\":3,\"tags\":[\"alpha\",\"beta\",\"gamma\"]}}")
	return doc
}

main :: proc() {
	doc := make_document()
	defer strings.builder_destroy(&doc)
	text := strings.to_string(doc)
	total: i64
	for _ in 0 ..< 100 {
		parsed, err := json.parse(text, .JSON, true)
		if err != .None {
			panic("json parse failed")
		}
		total += checksum(parsed)
		json.destroy_value(parsed)
	}
	fmt.println(total)
}
