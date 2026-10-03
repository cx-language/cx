// Iterator and closure chains, matching mapfilter.cx.
let divisor = 7
let factor = 3

var numbers: [Int] = []
numbers.reserveCapacity(4_000_000)
for i in 0..<4_000_000 {
    numbers.append(i)
}
var total: Int64 = 0
for _ in 0..<40 {
    for x in numbers.lazy.filter({ $0 % divisor == 0 }).map({ $0 * factor + 1 }).filter({ $0 % 2 == 0 }) {
        total += Int64(x)
    }
}
let offset = 1
let adjust = { (n: Int) in n * factor + offset }
for i in 0..<8_000_000 {
    total += Int64(adjust(i))
}
print(total)
