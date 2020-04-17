
module IOMonad =
    type IO<'a> = 
        private | IO of (unit -> 'a)
        static member (>>=) (IO r,f:'a -> _ IO) = 
            IO (r >> (f >> (fun (IO r) -> r ())))

    module IO =
        let wrap = IO
        let map f (IO r) = IO (r >> f)
        let unwrap (IO r) = r ()
        

    let readLine = IO.wrap stdin.ReadLine
    let readInt = readLine |> IO.map int
    let readFloat = readLine |> IO.map float

    let printLine str = IO.wrap (fun () -> stdout.WriteLine (string str))

    let action =
        printLine "== Reader Monad =="
        >>= (fun () ->
            let read =
                readLine 
                >>= (fun str -> 
                    readInt
                    >>= (fun i -> 
                        readFloat |> IO.map 
                            (fun f -> str,i,f)))

            printLine "Input string, int and float:"
            >>= (fun () -> 
                read
                >>= (fun x -> printLine (string x))))

module StateMonad =
    type State<'a,'s> =
        private | State of ('s -> ('a * 's))
        static member (>>=) (State (a:'s->'a*'s),f:'a->State<'b,'s>) : State<'b,'s> = 
            State (fun s ->
                let v,state = a s
                let (State p) = f v
                p state)
    
    module State =
        let wrap = State
        let map f (State s) = State (fun state ->
            let a,state2 = s state
            f a,state2)
        let ret = map (fun x -> x)
        let eval a (State s) = s a |> fst

    let inc = State.wrap (fun x -> x + 1,x + 1)
    let pushStack (value:'a) = State.wrap (fun (stack:'a list) -> (),value::stack)
    let popStack () : State<'a option,'a list> = 
        State.wrap (function | value :: remainder -> (Some value),remainder | [] -> None,[])

    let action =
        IOMonad.printLine "== State Monad =="
        >>= (fun () ->
            let state =
                inc
                >>= (fun _ ->
                    inc
                    >>= (fun _ ->
                        inc
                        |> State.ret))
                |> State.eval 0
            IOMonad.printLine (string state)
            >>= (fun () ->
                let myStack = 
                    pushStack 1
                    >>= (fun () -> 
                        pushStack 2
                        >>= (fun () -> 
                            pushStack 3
                            >>= (fun () -> popStack ())))
                IOMonad.printLine ("Stack Top:" + (string (State.eval [] myStack)))))

module ErrorMonad =
    type Error<'a,'e> = 
        | OK of 'a
        | Err of 'e
        static member (>>=) (e:Error<'a,'e>,f:'a->Error<'b,'e>) : Error<'b,'e> =
            match e with
            | OK a -> f a
            | Err e -> Err e

    let action =
        IOMonad.printLine "== Error Monad =="
        >>= (fun () ->
            let err : Error<int,exn> = 
                OK 1
                >>= (fun x ->
                    OK (1+x))
            IOMonad.printLine (string err))



// Reader Monad
// Writer Monad

ErrorMonad.action
>>= (fun () ->
    StateMonad.action
    >>= (fun () ->
        IOMonad.action))
|> IOMonad.IO.unwrap
