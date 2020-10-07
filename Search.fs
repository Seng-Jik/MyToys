
type Fact =
    abstract IsSame : Fact -> bool
    
type Atom<'a when 'a : equality> (value: 'a) =
    member x.Value = value
    interface Fact with
        member x.IsSame y = 
            if y :? Atom<'a> then
                (y :?> Atom<'a>).Value = value
            else false
        
type Friend (a: Atom<string>, b: Atom<string>) =
    member x.A = a
    member x.B = b
    interface Fact with
        override x.IsSame y = 
            if y :? Friend then
                (((y :?> Friend).A :> Fact).IsSame(a :> Fact) && ((y :?> Friend).B :> Fact).IsSame(b :> Fact)) ||
                (((y :?> Friend).A :> Fact).IsSame(b :> Fact) && ((y :?> Friend).B :> Fact).IsSame(a :> Fact))
            else false

let peter = Atom "Peter"
let bill = Atom "Bill"
let gates = Atom "Gates"

let facts : Fact list = [
    peter :> Fact
    bill :> Fact
    
    Friend (peter, bill) :> Fact
]

let IsTrue (facts: Fact seq) (factToCheck: #Fact) =
    facts
    |> Seq.exists (fun x -> x.IsSame factToCheck)
    
printfn "%A" (IsTrue facts (Friend (bill, peter)))
printfn "%A" (IsTrue facts (Friend (bill, gates)))
