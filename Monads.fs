

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
        printLine "== IO Monad =="
        >>= (fun () -> printLine "Input string, int and float:")
        >>= (fun () -> readLine)
        >>= (fun str -> readInt |> IO.map (fun x -> str,x))
        >>= (fun (str,i) -> readFloat |> IO.map (fun x -> str,i,x))
        >>= (fun x -> printLine (string x))

module StateMonad =
    type State<'a,'s> =
        private | State of ('s -> ('a * 's))
        static member (>>=) (State (a:'s->'a*'s),f:'a->State<'b,'s>) : State<'b,'s> = 
            State (fun s -> let v,state = a s in let (State p) = f v in p state)
    
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
            inc
            >>= (fun _ -> inc)
            >>= (fun _ -> inc)
            |> State.ret
            |> State.eval 0
            |> string
            |> IOMonad.printLine)
        >>= (fun () ->
            pushStack 1
            >>= (fun () -> pushStack 2)
            >>= (fun () -> pushStack 3)
            >>= (fun () -> popStack ())
            |> State.eval []
            |> string
            |> (+) "Stack Top:"
            |> IOMonad.printLine)

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
            OK 1
            >>= (fun x -> OK (1+x))
            |> string
            |> IOMonad.printLine)


module ReaderMonad =
    type Reader<'a,'src> = 
        private | Reader of  ('src -> 'a)
        static member (>>=) (Reader (r:'src->'a),f:'a->Reader<'b,'src>) : Reader<'b,'src> =
            Reader (fun src -> let (Reader b) = src |> r |> f in b src)

    module Reader =
        let wrap = Reader
        let eval src (Reader r) = r src
        let map (f:'a->'b) (Reader r) : Reader<'b,_> = r >> f |> Reader

    let action =
        IOMonad.printLine "== Reader Monad =="
        >>= (fun () ->
            Reader.wrap (fun (x:string) -> x.[0])
            >>= (fun a -> Reader.wrap (fun (x:string) -> x.[1]) |> Reader.map (fun b -> a,b))
            |> Reader.eval "MO"
            |> string
            |> IOMonad.printLine)

module WriterMonad =
    type Writer<'a,'w when 'w : (static member (+) : 'w*'w -> 'w)> = 
        | Writer of 'a * 'w
        static member inline (>>=) (Writer (a,w),f:'a->Writer<'b,'w2>) : Writer<'b,'w2> =
            let (Writer (a2,w2)) = f a in Writer (a2,w + w2)

    module Writer =
        let inline wrap v initLog = Writer (v,initLog)
        let inline map (f:'a -> 'b) (Writer (a,b)) = Writer (f a,b)
        let inline snd (Writer (_,s)) = s
        let inline eval (Writer (a,_)) = a

    let action = 
        IOMonad.printLine "== Writer Monad =="
        >>= (fun () -> 
            Writer.wrap 0 "zero|"
            >>= (fun x1 -> Writer.wrap 1 "one" |> Writer.map (fun x -> x1,x))
            |> Writer.map (fun (x1,x2) -> x1,x2)
            |> string
            |> IOMonad.printLine)


ErrorMonad.action
>>= (fun () -> StateMonad.action)
>>= (fun () -> ReaderMonad.action)
>>= (fun () -> WriterMonad.action)
>>= (fun () -> IOMonad.action)
|> IOMonad.IO.unwrap

