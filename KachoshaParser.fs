module Kachosha.Assemblier.Parser

type private Input = {
    src : char seq
    position : int
    
    row : uint32
    col : uint32
}

type private parsed<'a> = Result<'a * Input,exn>
type private parser<'a> = Input -> parsed<'a>

let private createInput (str:string) = 
    {
        src = Seq.cast str
        position = 0
        row = 0u
        col = 0u
    }

let private isNewLine c = c = '\n'

exception private EndingFound
exception private ConditionNotMatched

let private pred parser condition = fun input ->
    match parser input with
    | Ok (p,remainder) when condition p -> Ok (p,remainder)
    | _ -> Error ConditionNotMatched

let private anyChar : char parser = fun input ->
    match Seq.tryItem input.position input.src with
    | Some x ->
        let nextInput =
            match x with
            | x when isNewLine x ->
                {
                    input with
                        position = input.position + 1
                        row = input.row + 1u
                        col = 0u
                }
            | _ -> 
                {
                    input with
                        position = input.position + 1
                        col = input.col + 1u
                }
        Ok (x,nextInput)
    | None -> Error EndingFound
    
exception private LiteralNotMatched of string
let rec private literal expected : unit parser = fun input ->
    match expected with
    | "" -> Ok ((),input)
    | x -> 
        match pred anyChar (fun x -> x = expected.[0]) input with
        | Ok (_,input) -> literal expected.[1..] input
        | Error _ -> Error (LiteralNotMatched expected)
        
     
"aastupid"
|> createInput
|> literal "aastu"
|> printfn "%A"

