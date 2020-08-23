type Result<'a,'e> = 
| Ok of 'a
| Error of 'e

let bind : ('a -> Result<'b,'e>) -> Result<'a,'e> -> Result<'b,'e> =
    fun f a -> 
        match a with
        | Ok v -> f v
        | Error e -> Error e
        
let map : ('a -> 'b) -> Result<'a,'e> -> Result<'b,'e> =
    fun f -> bind (f >> Ok)
    
type ResultBuilder () =
    member _.Bind (a,f) = bind f a
    member _.Return a = Ok a
    member _.ReturnFrom a = a
    member _.TryWith (a,ef) =
        match a with
        | Ok a -> Ok a
        | Error e -> ef e
    member _.TryFinally (a,f) =
        f ()
        a
    member _.Delay x = x ()
    member _.Zero () = Ok ()
    member _.Combine (_:Result<unit,'e>,b) = b

let result = ResultBuilder ()

let throw x = Error x
let success x = Ok x

result {
    let! x = success 100
    printfn "%A" x
    
    try
        let! x = success 1000
        printfn "%A" x
        let! _ = throw "fuck"
        printfn "Should not here."
    with x ->
        printfn "%s has been thrown." x
        
    try
        let! a = success 1000
        printfn "%A" a
        do! throw "fucker"
    finally 
        printfn "Finally"
}
|> printfn "Final Result:%A"
