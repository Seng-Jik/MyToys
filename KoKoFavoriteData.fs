open System.IO

let reads bytes (x: BinaryReader) =
    x.ReadBytes (bytes) |> Array.map uint64
    |> Array.fold (fun state x -> (state <<< 8) ||| x) 0UL
    
let readInt32 = reads 4 >> int
let readUInt32 = reads 4 >> uint
let readUInt64 = reads 8

let readData x =
    use reader = new BinaryReader (new FileStream(x, FileMode.Open))
    let count = readInt32 reader
    [ for _ in 0..count - 1 ->
        let len = readUInt32 reader
        let spider =
            [| for _ in 0..(int len-1) -> reader.ReadChar () |]
            |> fun x -> new System.String (x)
        let id = readUInt64 reader
        spider, id ]
        
let write bytes (x: uint64) (w: BinaryWriter) =
    [| for i in 0..bytes-1 -> 0xFFUL &&& (x >>> (i * 8)) |]
    |> Array.map byte
    |> Array.rev
    |> w.Write
    
let inline write32 w (x: ^T) = write 4 (uint64 x) w
let inline write64 w (x: ^T) = write 8 (uint64 x) w

let writeData (out: string) (x: (string * uint64) list) =
    use writer = new BinaryWriter (new FileStream (out, FileMode.CreateNew))
    write32 writer <| List.length x
    for i in x do
        String.length (fst i) |> uint32 |> write32 writer
        writer.Write((fst i).ToCharArray())
        write64 writer <| snd i

