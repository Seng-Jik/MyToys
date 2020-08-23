[<NoComparison>]
[<NoEquality>]
type Reader<'a,'r> = Reader of ('a -> 'r)

let bind : ('a -> Reader<'a,_>) -> Reader<'a,_> -> Reader<'a,_> =
    fun f (Reader a) -> 
        Reader (fun src ->
            let (Reader b) = a src |> f
            b src)
            
let ask<'a> () : Reader<'a,'a> = Reader (fun src -> src)

let run : 'a -> Reader<'a,'r> -> 'r =
    fun s (Reader f) -> f s

type ReaderBuilder () =
    member _.Bind (a,f) = bind f a
    member _.Return x = Reader (fun _ -> x)
    
let reader = ReaderBuilder ()

let hello : Reader<string,string> =
    reader {
        let! name = ask<string>()
        return ("Hello, " + name + "!")
    }
    
let bye : Reader<string,string> =
    reader {
        let! name = ask<string>()
        return ("Bye, " + name + "!")
    }
    
let convo : Reader<string,string> = 
    reader {
        let! c1 = hello
        let! c2 = bye
        return (c1 + c2)
    }
    
printfn "%A" (run "Jesus" convo)
