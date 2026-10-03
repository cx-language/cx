// JSON parse and walk, matching jsonparse.cx. Foundation's parser keeps
// numbers as NSNumber; integral values read back as integers the way
// JsonValue.Integer and Number do (this document spells every integral
// number without a decimal point, so value and spelling agree).
import Foundation

func checksum(_ value: Any) -> Int64 {
    if CFGetTypeID(value as CFTypeRef) == CFBooleanGetTypeID() {
        return (value as! Bool) ? 1 : 0
    }
    switch value {
    case let s as String:
        return Int64(s.utf8.count)
    case let n as NSNumber:
        let d = n.doubleValue
        if d == d.rounded(.towardZero) {
            return Int64(d)
        }
        return Int64(d * 1000)
    case is NSNull:
        return 0
    case let items as [Any]:
        var sum: Int64 = 0
        for item in items {
            sum += checksum(item)
        }
        return sum
    case let members as [String: Any]:
        var sum: Int64 = 0
        for (key, item) in members {
            sum += Int64(key.utf8.count) + checksum(item)
        }
        return sum
    default:
        fatalError("unexpected JSON value")
    }
}

func makeDocument() -> String {
    var doc = "{\"users\":["
    for i in 0..<5000 {
        if i > 0 {
            doc += ","
        }
        // The score is exactly i * 1.5; spell integral values without a
        // decimal point so they parse as integers, like the other ports.
        let tripled = i * 3
        let score = tripled % 2 == 0 ? "\(tripled / 2)" : "\(tripled / 2).5"
        doc += "{\"id\":\(i),\"name\":\"user\(i)\",\"score\":\(score),\"active\":\(i % 2 == 0)}"
    }
    doc += "],\"meta\":{\"version\":3,\"tags\":[\"alpha\",\"beta\",\"gamma\"]}}"
    return doc
}

let text = makeDocument()
let data = Data(text.utf8)
var total: Int64 = 0
for _ in 0..<100 {
    let parsed = try! JSONSerialization.jsonObject(with: data)
    total += checksum(parsed)
}
print(total)
