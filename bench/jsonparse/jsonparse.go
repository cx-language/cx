// JSON parse and walk, matching jsonparse.cx. encoding/json's generic
// value keeps every number as a json.Number, so the walk can tell integer
// tokens from fractions the way JsonValue.Integer and Number do.
package main

import (
	"encoding/json"
	"fmt"
	"strconv"
	"strings"
)

func checksum(value any) int64 {
	switch v := value.(type) {
	case string:
		return int64(len(v))
	case json.Number:
		if strings.ContainsAny(v.String(), ".eE") {
			f, err := v.Float64()
			if err != nil {
				panic(err)
			}
			return int64(f * 1000)
		}
		n, err := v.Int64()
		if err != nil {
			panic(err)
		}
		return n
	case bool:
		if v {
			return 1
		}
		return 0
	case nil:
		return 0
	case []any:
		var sum int64
		for _, item := range v {
			sum += checksum(item)
		}
		return sum
	case map[string]any:
		var sum int64
		for key, item := range v {
			sum += int64(len(key)) + checksum(item)
		}
		return sum
	default:
		panic(fmt.Sprintf("unexpected JSON value %T", value))
	}
}

func makeDocument() string {
	var doc strings.Builder
	doc.WriteString(`{"users":[`)
	for i := 0; i < 5000; i++ {
		if i > 0 {
			doc.WriteByte(',')
		}
		score := float32(i) * 1.5
		fmt.Fprintf(&doc, `{"id":%d,"name":"user%d","score":%s,"active":%t}`,
			i, i, strconv.FormatFloat(float64(score), 'g', -1, 32), i%2 == 0)
	}
	doc.WriteString(`],"meta":{"version":3,"tags":["alpha","beta","gamma"]}}`)
	return doc.String()
}

func main() {
	text := makeDocument()
	var total int64
	for range 100 {
		dec := json.NewDecoder(strings.NewReader(text))
		dec.UseNumber()
		var parsed any
		if err := dec.Decode(&parsed); err != nil {
			panic(err)
		}
		total += checksum(parsed)
	}
	fmt.Println(total)
}
