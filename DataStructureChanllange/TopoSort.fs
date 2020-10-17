// 拓扑排序(输出所有可能的结果)

[<NoComparison>]
type Graph<'node> = {
    verticles : 'node list
    edges : ('node * 'node) list
}

module Graph =
    let removeVertex v g = {
        verticles = List.except [v] g.verticles
        edges = List.filter (fst >> (<>) v) g.edges
    }


    let rec topoSorts g =
        let vertexNoIndegree = 
            let vertexWithIndegree = List.map snd g.edges
            List.except vertexWithIndegree g.verticles
            
        match g.verticles, vertexNoIndegree with
        | [], [] -> [[]]
        | _, [] -> []
        | _, vertexToRemove ->
            vertexToRemove
            |> List.collect (fun vertexToRemove ->
                topoSorts <| removeVertex vertexToRemove g
                |> List.map (List.append [vertexToRemove]))
            
            
let a, b, c, d, e = "a", "b", "c", "d", "e"

let g = {
    verticles = [ a; b; c; d; e ]
    edges = [ a, b; a, c; a, e;
              b, c; e, d; c, d ]
}

Graph.topoSorts g |> List.iter (printfn "%A")
