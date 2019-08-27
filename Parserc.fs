module Parserc

type str = char[]
type parsed = ('a * str) option
type Parser<'a> = str -> parsed

let characterInCondition condition : Parser<char> = fun input ->
    match Array.tryHead input with
    | Some x when condition x -> (x,Array.tail input)
    | _ -> None

let characterInRange min max = characterInCondition (fun x -> x >= min && x <= max)
let character x = characterInRange x x
let littleLetter = characterInRange 'a' 'z'
let bigLetter = characterInRange 'A' 'Z'
let numberCharacter = characterInRange '0' '9'
