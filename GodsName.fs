
let a = seq {'A'..'Z'}
let b = Seq.map string a

let f (s:string seq) : string seq = 
    Seq.allPairs b s
    |> Seq.map (fun (a,b) -> a + b)

let rec r next i =
    match i with
    | 1 -> next
    | i ->
        r (f next) (i-1)
        
let condition (name:string) : bool =
    name
    |> Seq.countBy (fun x -> x)
    |> Seq.exists (fun (a,b) -> b > 3)
    |> not
        
let godsName =
    r b 9
    |> Seq.filter condition

let fs = System.IO.File.CreateText "GodsName.txt"
godsName
|> Seq.iter (fun name ->
    fprintfn fs "%s" name
    printfn "%s" name)
