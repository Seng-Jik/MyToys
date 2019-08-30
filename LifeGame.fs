type Cell =
| Live
| Died

type Map = Cell[][]

let myMap =
    let width = 150
    let height = 58
    Array.init height (fun y ->
        Array.init width (fun x ->
            if x = width / 2 || y = height / 2 then Live
            else Died))

let printMap =
    Array.iter (fun line ->
        line
        |> Array.iter (
            function
            | Live -> '*'
            | Died -> ' '
            >> printf "%c")
        printfn "")

let clamp v small big =
    if v < small then small
    else if v > big then big
    else v

let slice first last array =
    let s = clamp first 0 (-1 + Array.length array)
    let e = clamp last 0 (-1 + Array.length array)
    array.[s..e]

let nextMap map =
    map
    |> Array.Parallel.mapi (fun y line ->
        line
        |> Array.mapi (fun x _ ->
            let cur = map.[y].[x]
            let neibours =
                map
                |> slice (y-1) (y+1)
                |> Array.collect (slice (x-1) (x+1))
                |> Array.filter ((=) Live)
                |> Array.length
                |> function
                | count when cur = Live -> count - 1
                | count -> count
            match neibours with
            | 3 -> Live
            | 2 -> cur
            | _ -> Died))
        
let rec playGame map =
    System.Console.Clear ()
    printMap map
    System.Threading.Thread.Sleep 20
    playGame (nextMap map)

playGame myMap
