let x = [
  "let x = ["
  ""
  "]"
  ""
  "let f (x: string) = \"  \\\"\" + x.Replace(\"\\\\\",\"\\\\\\\\\").Replace(\"\\\"\", \"\\\\\\\"\") + \"\\\"\""
  ""
  "x.[0] :: List.map f x @ x.[1..]"
  "|> List.iter (printfn \"%s\")"

]

let f (x: string) = "  \"" + x.Replace("\\","\\\\").Replace("\"", "\\\"") + "\""

x.[0] :: List.map f x @ x.[1..]
|> List.iter (printfn "%s")



