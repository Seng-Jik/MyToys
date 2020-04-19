
let Do x = fun _ -> x
let inline Zip x = fun a -> x => (fun b -> a,b)

module IOMonad =
    type IO<'a> = 
        private | IO of (unit -> 'a)
        static member (>>=) (IO r,f:'a -> _ IO) = 
            IO (r >> (f >> (fun (IO r) -> r ())))
        static member (=>) (IO r,f:'a->'b) = 
            IO (r >> f)

    module IO =
        let wrap = IO
        let unwrap (IO r) = r ()

        type IOBuilder () =
            member _.Bind (a:'a IO,f) = a >>= f
            member _.Zero () = IO ignore
        
        
    let io = IO.IOBuilder ()

    let readLine = IO.wrap stdin.ReadLine
    let readInt = readLine => int
    let readFloat = readLine => float

    let printLine str = IO.wrap (fun () -> stdout.WriteLine (string str))

    let action =
        printLine "== IO Monad =="
        >>= Do (printLine "Input string, int and float:")
        >>= Do readLine
        >>= Zip readInt
        >>= Zip readFloat
        => string
        >>= printLine

    let actionByComputationExpression =
        io {
            do! printLine "== IOMonad =="
            do! printLine "Input string, int and float:"
            let! line = readLine
            let! i = readInt
            let! f = readFloat
            let r = (line,i),f
            do! printLine (string r)
        }

module StateMonad =
    type State<'a,'s> =
        private | State of ('s -> ('a * 's))
        static member (>>=) (State (a:'s->'a*'s),f:'a->State<'b,'s>) : State<'b,'s> = 
            State (fun s -> let v,state = a s in let (State p) = f v in p state)
        static member (=>) (State s,f:'a -> 'b) =
            State (fun state ->
                let a,state2 = s state
                f a,state2)
    
    module State =
        let wrap = State
        let eval a (State s) = s a |> fst

        type StateBuilder () =
            member _.Bind (a:State<_,_>,f) = a >>= f
            member _.Zero () = State (fun x -> x,x)
            member _.Return x = x
    
    let state = State.StateBuilder ()

    let inc = State.wrap (fun x -> x + 1,x + 1)
    let pushStack (value:'a) = State.wrap (fun (stack:'a list) -> (),value::stack)
    let popStack () : State<'a option,'a list> = 
        State.wrap (function | value :: remainder -> (Some value),remainder | [] -> None,[])

    let action =
        IOMonad.printLine "== State Monad =="
        >>= Do (
            inc
            >>= Do inc
            >>= Do inc
            |> State.eval 0
            |> string
            |> IOMonad.printLine)
        >>= Do (
            pushStack 1
            >>= Do (pushStack 2)
            >>= Do (pushStack 3)
            >>= Do (popStack ())
            |> State.eval []
            |> string
            |> (+) "Stack Top:"
            |> IOMonad.printLine)

    open IOMonad
    let actionWithComputationExpression =
        io {
            do! printLine "== State Monad =="
            do!
                state {
                    let! _ = inc
                    let! _ = inc
                    let! _ = inc
                    ()
                }
                |> State.eval 0
                |> string
                |> printLine

            do!
                state {
                    let! _ = pushStack 2
                    let! _ = pushStack 3
                    return popStack ()
                }
                |> State.eval []
                |> string
                |> (+) "Stack Top:"
                |> IOMonad.printLine
        }

module ErrorMonad =
    type Error<'a,'e> = 
        | OK of 'a
        | Err of 'e
        static member (>>=) (e:Error<'a,'e>,f:'a->Error<'b,'e>) : Error<'b,'e> =
            match e with
            | OK a -> f a
            | Err e -> Err e

    type ErrorBuilder () =
        member _.Bind (a:Error<'a,'e>,f) = a >>= f
        member _.Return x = x

    let error = ErrorBuilder ()

    let action =
        IOMonad.printLine "== Error Monad =="
        >>= Do (
            OK 1
            >>= fun x -> OK (1+x)
            |> string
            |> IOMonad.printLine)
        >>= Do(
            OK 1
            >>= fun _ -> Err 0
            |> string
            |> IOMonad.printLine)

    open IOMonad
    let actionWithComputationExpression =
        io {
            do! printLine "== Error Monad =="
            do!
                error {
                    let! x = OK 1
                    return OK (x + 1)
                }
                |> string
                |> printLine
            do! 
                error {
                    let! _ = OK 1
                    let! y = Err 0
                    let z = y + 1
                    return OK z
                }
                |> string
                |> printLine
        }


module ReaderMonad =
    type Reader<'a,'src> = 
        private | Reader of  ('src -> 'a)
        static member (>>=) (Reader (r:'src->'a),f:'a->Reader<'b,'src>) : Reader<'b,'src> =
            Reader (fun src -> let (Reader b) = src |> r |> f in b src)
        static member (=>) (Reader r,f:'a -> 'b) =
            r >> f |> Reader

    module Reader =
        let wrap = Reader
        let eval src (Reader r) = r src

        type ReaderBuilder () =
            member _.Bind (a:Reader<_,_>,f) = a >>= f
            member _.Return x = x
    let reader = Reader.ReaderBuilder ()

    let action =
        IOMonad.printLine "== Reader Monad =="
        >>= Do (
            Reader.wrap (fun (x:string) -> x.[0])
            >>= Zip (Reader.wrap (fun (x:string) -> x.[1]))
            |> Reader.eval "MO"
            |> string
            |> IOMonad.printLine)

    open IOMonad
    let actionWithComputationExpression =
        io {
            do! printLine "== Reader Monad =="
            do!
                reader {
                    let! a = Reader.wrap (fun (x:string) -> x.[0])
                    let! b = Reader.wrap (fun (x:string) -> x.[1])
                    return Reader.wrap (fun _ -> a,b)
                }
                |> Reader.eval "MO"
                |> string
                |> IOMonad.printLine
        }

module WriterMonad =
    type Writer<'a,'w when 'w : (static member (+) : 'w*'w -> 'w)> = 
        | Writer of 'a * 'w
        static member inline (>>=) (Writer (a,w),f:'a->Writer<'b,'w2>) : Writer<'b,'w2> =
            let (Writer (a2,w2)) = f a in Writer (a2,w + w2)
        static member inline (=>) (Writer (a,b),f:'a -> 'b) =
            Writer (f a,b)

    module Writer =
        let inline wrap v initLog = Writer (v,initLog)
        let inline snd (Writer (_,s)) = s
        let inline eval (Writer (a,_)) = a

        type WriterBuilder () =
            member _.Bind (a:Writer<_,_>,f) = a >>= f
            member _.Return x = x
    let writer = Writer.WriterBuilder ()

    let action = 
        IOMonad.printLine "== Writer Monad =="
        >>= Do (
            Writer.wrap 0 "zero|"
            >>= Zip (Writer.wrap 1 "one")
            |> string
            |> IOMonad.printLine)

    open IOMonad
    let actionWithComputationExpression =
        io {
            do! printLine "== Writer Monad =="
            do!
                writer {
                    let! x = Writer.wrap 0 "zero|"
                    return Writer.wrap (x,1) "one"
                }
                |> string
                |> IOMonad.printLine
        }


IOMonad.printLine "======== Monad Directly ========"
>>= Do ErrorMonad.action
>>= Do StateMonad.action
>>= Do ReaderMonad.action
>>= Do WriterMonad.action
>>= Do IOMonad.action

>>= Do (IOMonad.printLine "")
>>= Do (IOMonad.printLine "======== Monad With Computation Expression ========")
>>= Do ErrorMonad.actionWithComputationExpression
>>= Do StateMonad.actionWithComputationExpression
>>= Do ReaderMonad.actionWithComputationExpression
>>= Do WriterMonad.actionWithComputationExpression
>>= Do IOMonad.actionByComputationExpression

|> IOMonad.IO.unwrap


