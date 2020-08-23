
[<NoComparison>]
[<NoEquality>]
type State<'T,'State> = State of ('State -> 'State*'T)

let bind : ('a -> State<'b,'State>) -> State<'a,'State> -> State<'b,'State> =
    fun f (State a) -> 
        State (fun state ->
            let nextState,v = a state
            let (State p) = f v
            p nextState)
        
let map : ('a -> 'b) -> State<'a,'State> -> State<'b,'State> =
    fun f (State a) ->
        State (fun state ->
            let nextState,v = a state
            nextState,f v)
            
let run : 'State -> State<'a,'State> -> 'a =
    fun s (State f) -> f s |> snd
    
type StateBuilder () =
    member _.Bind (a,f) = bind f a
    member _.ReturnFrom x = x
    
let state = StateBuilder ()

let inc = State (fun x -> let x = x + 1 in x,x)
let dec = State (fun x -> let x = x - 1 in x,x)

state {
    let! a = inc
    printfn "%A" a
    let! a = inc
    printfn "%A" a
    let! a = inc
    printfn "%A" a
    let! a = dec
    printfn "%A" a
    let! a = dec
    printfn "%A" a
    let! a = dec
    printfn "%A" a
    return! dec
}
|> run 0
|> ignore
