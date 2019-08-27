module Parserc

type str = char[]
type parsed<'a> = ('a * str) option
type parser<'a> = str -> 'a parsed

let characterInCondition condition = fun input ->
    match Array.tryHead input with
    | Some x when condition x -> Some (x,Array.tail input)
    | _ -> None

let characterInRange min max = characterInCondition (fun x -> x >= min && x <= max)
let character x = characterInRange x x
let littleLetter = characterInRange 'a' 'z'
let bigLetter = characterInRange 'A' 'Z'
let numberCharacter = characterInRange '0' '9'

let (<||>) a b = 
    a
    >> function
    | Some (result,reminder) -> Some (Choice1Of2(result),reminder)
    | None ->
         match b input with
         | Some (result,reminder) -> Some (Choice2Of2(result),reminder)
         | None -> None
         
let (<|>) a b = 
    >> a <||> b
    >> function
    | Some (result,reminder) ->
        match result with
        | Choice1Of2 x -> x
        | Choice2Of2 x -> x
    | None -> None
         
let (<+>) a b = 
    a
    >> function
    | Some (result1,reminder1) ->
        match b reminder1 with
        | Some (result2,reminder2) ->
            Some (result1,result2)
        | None -> None
    | None -> None
    
