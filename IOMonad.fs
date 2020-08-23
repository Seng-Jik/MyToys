[<NoComparison>]
[<NoEquality>]
type IO<'a> = IO of (unit -> 'a)

let print x : IO<unit> = IO (fun () -> printf "%A" x)
let printLn x : IO<unit> = IO (fun () -> printfn "%A" x)
let scanLine : IO<string> = IO (fun () -> System.Console.ReadLine ())

let bind : ('a -> IO<'b>) -> IO<'a> -> IO<'b> =
    fun f (IO a) -> IO (fun () -> let (IO x) = (a >> f) () in x ())

let map : ('a -> 'b) -> IO<'a> -> IO<'b> =
    fun f -> bind (fun v -> IO (fun () -> f v))
 
let run : IO<'a> -> 'a =
    fun (IO f) -> f ()
 
type IOBuilder () =
    member _.Bind (a,f) = bind f a
    member _.Return x = IO (fun () -> x)
    member _.ReturnFrom x = x
    member _.Zero () = IO (fun () -> ())

let io = IOBuilder ()

io {
    do! printLn "Hello, world!"
    do! print 1
    do! print 2
    do! print 3
    let! abc = scanLine
    do! printLn abc
    let! _ = scanLine
    ()
}
|> run
