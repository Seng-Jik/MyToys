
type Writer<'a,'Log> = Writer of 'a * 'Log list

let bind : ('a -> Writer<'b,'Log>) -> Writer<'a,'Log> -> Writer<'b,'Log> =
    fun f (Writer (v,log)) ->
        let (Writer (v2,log2)) = f v
        Writer (v2,List.append log log2)
        
let map : ('a -> 'b) -> Writer<'a,'Log> -> Writer<'b,'Log> =
    fun f (Writer (a,log)) -> Writer (f a,log)
    
let log : 'Log -> Writer<unit,'Log> =
    fun s -> Writer ((),[s])
    
let getLog : Writer<'a,'Log> -> 'Log list =
    fun (Writer (_,l)) -> l
    
type WriterBuilder () =
    member _.Bind (a,f) = bind f a
    member _.ReturnFrom x = x
    member _.Yield x = Writer (x,[])
    member _.For (e,f) = 
        let log =
            Seq.map (fun x -> 
                let (Writer (_,log)) = f x
                log) e
            |> Seq.toList
            |> List.concat
        let v = Seq.last e
        Writer (v,log)
    [<CustomOperation("tell")>]
    member _.Tell (prev,x) = bind (fun a ->  Writer (a,[x])) prev
    
let writer = WriterBuilder ()

writer {
    tell "123"
    tell "456"
    tell "789"
    tell "--END--"
}
|> getLog
|> printfn "%A"
