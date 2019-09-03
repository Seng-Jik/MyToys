module Parserc

type str = char[]
type parsed<'a> = ('a * str) option
type parser<'a> = str -> 'a parsed

module Parsed =
    let map (f:'a->'b) : 'a parsed -> 'b parsed =
        Option.map (fun (a,input) -> f a,input)
    let ignore = map ignore

    let fst<'a,'b> : ('a*'b) parsed -> 'a parsed = map fst
    let snd<'a,'b> : ('a*'b) parsed -> 'b parsed = map snd

let (<||||>) (a:'a parser) (b:'b parser) : Choice<'a,'b> parser = fun input ->
    input
    |> a
    |> function
    | Some (result,reminder) -> Some (Choice1Of2(result),reminder)
    | None ->
         match b input with
         | Some (result,reminder) -> Some (Choice2Of2(result),reminder)
         | None -> None
         
let (<|>) (a:'a parser) (b:'a parser) : 'a parser =
    a <||||> b
    >> function
    | Some (result,reminder) ->
        match result with
        | Choice1Of2 x -> Some (x,reminder)
        | Choice2Of2 x -> Some (x,reminder)
    | None -> None
         
let (<+>) (a:'a parser) (b:'b parser) : ('a * 'b) parser = 
    a
    >> function
    | Some (result1,reminder1) ->
        match b reminder1 with
        | Some (result2,reminder2) ->
            Some ((result1,result2),reminder2)
        | None -> None
    | None -> None

let (<@+>) a b = a <+> b >> Parsed.fst

let (<+@>) a b = a <+> b >> Parsed.snd

    
let rec oneOrMore p = 
    p
    >> function
    | Some (result,reminder) -> 
       oneOrMore p reminder
       |> function
       | Some (results,finalReminder) ->
           Some(result::results,finalReminder)
       | None -> Some ([result],reminder)
    | None -> None
    
let zeroOrMore p = fun input ->
    input
    |> oneOrMore p
    |> function
    | Some x -> Some x
    | None -> Some ([],input)


let literal (expected:string) = 
    let rec impl expected input =
        if Array.isEmpty expected then
            Some ((),input)
        else
            match Array.tryHead input with
            | Some x when x = Array.head expected ->
                impl (Array.tail expected) (Array.tail input)
            | _ -> None
    impl (expected.ToCharArray())


let pred condition parser = fun input ->
    match parser input with
    | Some (p,remainder) when condition p -> Some (p,remainder)
    | _ -> None
    
let (>>>) first next = fun input ->
    match first input with
    | Some (parsed,remainder) -> next parsed remainder
    | None -> None
    
let (<<<) next first = first (>>>) next
    
let anyCharacter = fun input ->
    match Array.tryHead input with
    | Some x -> Some (x,Array.tail input)
    | _ -> None
    
let characterInCondition condition = pred condition anyCharacter

let characterInRange min max = characterInCondition (fun x -> x >= min && x <= max)
let character x = characterInRange x x
let littleLetter = characterInRange 'a' 'z'
let bigLetter = characterInRange 'A' 'Z'
let letter = bigLetter <|> littleLetter
let numberCharacter = characterInRange '0' '9'

let whitespace = characterInCondition (fun x -> x = ' ' || x = '\t' || x = '\010' || x = '\n' || x = '\r') >> Parsed.ignore


let quotedString = 
    character '\"' <+@> zeroOrMore (characterInCondition (fun x -> x <> '\"')) <@+> character '\"'
    >> Parsed.map List.toArray >> Parsed.map System.String

assert (quotedString ("\"Hello Joe!\"".ToCharArray()) = Some("Hello Joe!",[||]))

let identifier =
    character '_' <|> letter <+> zeroOrMore (character '_' <|> letter <|> numberCharacter <|> character '-')
    >> Parsed.map (fun (a,ls) -> a::ls |> List.toArray |> System.String)
   
module XMLParser =
    type XMLAttribute = {
        key : string
        value : string
    }
    
    type XMLElement = {
        tag : string
        attr : XMLAttribute list
        children : XMLElement list
    }

    let attribute = 
        identifier <+> zeroOrMore whitespace <+> character '=' <+> zeroOrMore whitespace <+> quotedString
        >> Parsed.map (fun x -> 
            let value = snd x
            let key = x |> fst |> fst |> fst |> fst
            { key = key;value = value })
        
    let attributes =
        zeroOrMore (oneOrMore whitespace <+> attribute)
        >> Parsed.map (List.map snd)
        
    let elementHeadBody =
        character '<' <+> zeroOrMore whitespace <+> identifier <+> attributes
        >> Parsed.map (fun x -> 
        {
            attr = snd x
            children = []
            tag = x |> fst |> snd 
        })
    let singleElement =
        elementHeadBody <@+> zeroOrMore whitespace <@+> character '/' <@+> zeroOrMore whitespace <@+> character '>'
        |> pred (fun x -> x.tag.StartsWith "/" |> not)
        
    let startElement =
        elementHeadBody <@+> zeroOrMore whitespace <@+> character '>'
        
    let endElement expectedName =
        character '<' <+@> zeroOrMore whitespace <+@> character '/' <+@> zeroOrMore whitespace <+@> identifier <@+> character '>'
        |> pred ((=) expectedName)
        
    let whitespaceWrapper parser =
        zeroOrMore whitespace <+@> parser <@+> zeroOrMore whitespace

    let rec parentElement input : XMLElement parsed =
        input 
        |> (startElement
        >>> (fun x -> 
            zeroOrMore (whitespaceWrapper element) <@+> zeroOrMore whitespace <@+> endElement x.tag
            >> Parsed.map (fun ls -> { x with children = ls })))

    and element input =
        let res =
            input 
            |> (singleElement <|> parentElement)
        res
        

        
    let parseXML =
        zeroOrMore (whitespaceWrapper element)
        

    let xml1 = """<top label="Top">
    <semi-bottom label="Bottom"/>
    <middle>
        <bottom label="Another bottom"/>
    </middle>
        </top>"""
        
    parentElement (xml1.ToCharArray())
    |> printfn "%A"
    
    
    (*printfn "%A" (attributes (" stupid    = \"oh my gooooood!\" super=\"fff\"".ToCharArray()))
    printfn "%A" (singleElement ("<super xx= \"abc\" yy = \"super\" />".ToCharArray()))*)


module Tests =
    let parse_joe = literal "Hello Joe!"
    assert(Some((),[||]) = parse_joe ("Hello Joe!".ToCharArray()))
    assert(Some((),(" Hello Robert!".ToCharArray())) = parse_joe ("Hello Joe! Hello Robert!".ToCharArray()))
    assert(None = parse_joe ("Hello Mike!".ToCharArray()))

    assert(character '_' ("_up".ToCharArray()) = Some('_',"up".ToCharArray()))
    assert(letter ("abc".ToCharArray()) = Some('a',"bc".ToCharArray()))
    assert(letter ("Abc".ToCharArray()) = Some('A',"bc".ToCharArray()))
    assert((zeroOrMore letter) ("aaAA11".ToCharArray()) = Some(['a';'a';'A';'A'],"11".ToCharArray()))

    assert(identifier ("i-am-an-identifier".ToCharArray()) = Some("i-am-an-identifier",[||]))
    assert(identifier ("not entirely an identifier".ToCharArray()) = Some("not"," entirely an identifier".ToCharArray()))
    assert(identifier ("!not at all an identifier".ToCharArray()) = None)

    let parse_ha = literal "ha"
    assert(Some([();();()],[||]) = oneOrMore parse_ha ("hahaha".ToCharArray()))
    assert(None = oneOrMore parse_ha ("ahah".ToCharArray()))
    assert(None = oneOrMore parse_ha [||])

    assert(Some([();();()],[||]) = zeroOrMore parse_ha ("hahaha".ToCharArray()))
    assert(Some([],"ahah".ToCharArray()) = zeroOrMore parse_ha ("ahah".ToCharArray()))
    assert(Some([],[||]) = zeroOrMore parse_ha [||])

module Brainfuck =

    type Instruction =
    | Inc
    | Dec
    | PInc
    | PDec
    | While of Instruction list
    | Input
    | Output

    let ins ch target =
        character ch >> Parsed.map (fun _ -> target)
        
    let simpleInstruction =
        ins '+' Inc <|>
        ins '-' Dec <|>
        ins '>' PInc <|>
        ins '<' PDec <|>
        ins ',' Input <|>
        ins '.' Output
        
    let dirtySimpleInstruction =
        simpleInstruction <||||> (characterInCondition (fun x-> x<>'['&&x<>']') >> Parsed.ignore)

    let cleanInstructions (instructions : Choice<Instruction,unit> list) : Instruction list =
        instructions
        |> List.collect (function
        | Choice1Of2 x -> [x]
        | Choice2Of2 ()-> [])

    let whileParser (brainfuckParser:Instruction list parser) : Instruction parser =
        character '[' <+@> brainfuckParser <@+> character ']'
        >> Parsed.map While
        

    let parseSingleInstruction (brainfuckParser:Instruction list parser) (input:str) : Choice<Instruction,unit> parsed =
        let whileParser = whileParser brainfuckParser
        let parser = whileParser <||||> dirtySimpleInstruction
        parser input
        |> Parsed.map (function
        | Choice1Of2 x -> Choice1Of2 x
        | Choice2Of2 x -> x)

    let rec parseBrainfuck (input:str) : Instruction list parsed =
        let single = parseSingleInstruction parseBrainfuck

        input
        |> zeroOrMore single
        |> Parsed.map cleanInstructions
        
        
    

    let bf = "[<----[][<<[++>]<<[]]--<------>>-]<<[>[>+>+<<-]>>[<<+>>-]<<<-]>>>++++++[<++++++++>-],<.>."
    printfn "%A" (parseBrainfuck (bf.ToCharArray()))
        
