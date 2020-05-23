module rec Jisp
//module rec Jisp.AST =

#nowarn "1178"

type JispNumber = System.Decimal

type JispExpr =
| Bind of BindExpr * JispExpr
| Value of JispValue
| TupleExpression of JispExpr list
| Identifier of string
| Apply of Apply

type JispValue =
| Lambda of JispFunc
| Number of JispNumber
| Tuple of JispValue list

type Apply = {
    Function : ApplyFunction
    Parameters : JispExpr list }
    
type ApplyFunction = 
| FunctionIdentifier of string
| FunctionExpression of JispExpr

type BindExpr = {
    Identifier : string
    Value : JispExpr }

type CustumFunc = {
    Parameters : string list
    Expression : JispExpr
    CurryingBindParameters : JispValue list }

type RuntimeFunc = Context -> JispExpr list -> Result<JispValue,exn>

type JispFunc = 
| CustumFunc of CustumFunc
| RuntimeFunc of RuntimeFunc
    
//module rec Jisp.Runtime =
type ContextRange = 
| Global
| Local
type Context = Map<string,JispValue * ContextRange>

exception IdenifierNotFound of string
exception CanNotCallTheValue
exception NotImpl
exception InvalidParameter
            

let getValueFromContext (context:Context) (id:string) : Result<JispValue,exn> =
    match context.TryFind id with
    | None -> Error (IdenifierNotFound id)
    | Some (x,_) -> Ok x
    
let evalParams context : JispExpr list -> Result<JispValue list,exn> = function
| a :: tail -> 
    eval context a 
    |> Result.bind (fun a ->
        evalParams context tail 
        |> Result.bind (fun t -> 
            Ok (a::t)))
| [] -> Ok []

let eval (context:Context) (ast:JispExpr) : Result<JispValue,exn> =
    match ast with
    | Value v -> 
        match v with
        | Lambda x -> Lambda x // 在这里捕获闭包内容
        | x -> x
        |> Ok
    | Bind (bind,expr) -> 
        eval context bind.Value
        |> Result.bind (fun x -> 
            eval (context.Add (bind.Identifier,(x,Local))) expr)
    | Identifier id -> getValueFromContext context id
    | TupleExpression expr -> evalParams context expr |> Result.map Tuple
    | Apply { Function = f; Parameters = param } -> 
        match f with
        | FunctionIdentifier id -> getValueFromContext context id
        | FunctionExpression expr -> eval context expr
        |> Result.bind (fun target ->
            match target with
            | Lambda x -> Ok x
            | _ -> Error CanNotCallTheValue)
        |> Result.bind (fun f -> 
            match f with
            | RuntimeFunc func -> func context param
            | CustumFunc func -> 
                evalParams context param
                |> Result.bind (fun param -> 
                    {   func with
                            CurryingBindParameters = 
                                 List.append func.CurryingBindParameters param }
                    |> fun func ->
                        if (List.length func.CurryingBindParameters) < (List.length func.Parameters) then
                            func |> CustumFunc |> Lambda |> Ok
                        else if (List.length func.CurryingBindParameters) > (List.length func.Parameters) then
                            Error InvalidParameter
                        else 
                            // TODO: 需要在这里支持自定义函数的调用
                            Error NotImpl))
            
            
module Library =
    
    let getNumber : JispValue -> Result<JispNumber,exn> = function
    | Number x -> Ok x
    | _ -> Error InvalidParameter
    
    let rtFunc = RuntimeFunc >> Lambda

    let arithmeticOperator id func =
        let numberFunction context : JispExpr list -> Result<JispValue,exn> = 
            evalParams context
            >> Result.bind (
                List.map getNumber
                >> List.reduce (fun (x:Result<JispNumber,exn>) (y:Result<JispNumber,exn>) -> 
                    match x with
                    | Ok x -> y |> Result.bind (fun y -> Ok (func x y))
                    | Error x -> Error x))
            >> Result.map Number
        id,(rtFunc numberFunction,Global)
        
    let booleanOperator id func =
        let booleanFunction context : JispExpr list -> Result<JispValue,exn> =
            evalParams context
            >> Result.bind (function
            | (Number _)::[] -> Error NotImpl   // TODO: 在这里需要支持柯里化
            | (Number a)::(Number b)::[] -> (if (func a b) then 1M else 0M) |> Number |> Ok
            | _ -> Error InvalidParameter)
        id,(rtFunc booleanFunction,Global)
        
    let notOperator context =
        evalParams context
        >> Result.bind (function
        | (Number a) :: [] -> (if a = 0M then 1M else 0M) |> Number |> Ok
        | _ -> Error InvalidParameter)
                            
    let defaultContext : Context = 
        Map.empty
            .Add(arithmeticOperator "+" (+))
            .Add(arithmeticOperator "-" (-))
            .Add(arithmeticOperator "*" (*))
            .Add(arithmeticOperator "/" (/))
            .Add(booleanOperator "=" (=))
            .Add(booleanOperator "!=" (<>))
            .Add(booleanOperator ">" (>))
            .Add(booleanOperator ">=" (>=))
            .Add(booleanOperator "<" (<))
            .Add(booleanOperator "<=" (<=))
            .Add(booleanOperator "<=" (<=))
            .Add(booleanOperator "|" (fun a b -> a <> 0M || b <> 0M))
            .Add(booleanOperator "&" (fun a b -> a <> 0M && b <> 0M))
            .Add("!",(rtFunc notOperator,Global))
        (*     
        • head 求list头部
        • tail 求list尾部
        • len 求list长度
        • concat 连接多个list
        • choose
        • collect
        • map
        • fold
        • filter
        • reduce
        • unfold
        • generate
        • readFile
        • Y
        • commandLine （这是一个list类型的值，用于读取命令行参数）*)
            
    
    
let testBindAndAdd = 
    Bind ({ Identifier = "sucker"; Value = Value (Number 1.0M) }, 
        (Bind({ Identifier = "sucker2"; Value = Value (Number 2.0M) },
            Apply {
                Function = FunctionIdentifier "-";
                Parameters = [
                    Value (Number 1.0M)
                    Value (Number 2.0M)]})))
        
printfn "TestBindAndAdd:%A" (eval Library.defaultContext testBindAndAdd)
    
let testTupleExpression = 
    TupleExpression 
        [
            Apply { Function = FunctionIdentifier "!=" ;Parameters = [ Value (Number 2M); Value (Number 1M)] }
            Value (Number 2.0M)
            Apply {
                Function = FunctionIdentifier "+";
                Parameters = [
                    Value (Number 1.0M)
                    Value (Number 2.0M)]}]
        
printfn "TestTupleExpression:%A" (eval Library.defaultContext testTupleExpression)
    

    