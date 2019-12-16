// 在FSharp上实现的Concept
// 其实就是泛型约束包了个函数而已
// 如果你正在学习如何使用F#泛型，可以参考


// 定义一个接口，参加到A的泛型约束中
type IA =
    abstract Stupid : unit -> unit

// 可以通过A约束测试的类型
type Impl () =
    interface IA with
        member x.Stupid () = ()

// 要求对类型参数'a和'b进行测试，同时要求b满足IA
type A<'a, 'b
    when 'a : struct
    and 'a : equality
    and 'a : unmanaged
    and 'b : not struct
    and 'b :> IA> = 
    'a * 'b

// 对类型^T进行测试，要求^T具有对^T的加法，且加法返回^T
let inline checkAddOperator 
    (x:^T 
        when ^T : (static member (+) : ^T * ^T -> ^T)
        and ^T : (member ToString : unit -> string)
        and ^T : unmanaged
        and ^T : equality
        and ^T :> obj) = 
    x

// 给定类型^T和^U，要求此两个类型可以相加
let inline checkAddable
    (x:^T
        when (^T or ^U) : (static member (+) : ^T * ^U -> 'R))
    (y:^U) =
    x,y,typeof<'R>.Name

// 可以与float32类型相加的类型
type AddA () = 
    static member (+) (_:AddA, _:float32) = ()

// 把约束A包装成函数
let checkDynamicConcept (a:A<'a,'b>) = a

// 各种测试
printfn "%A" (checkDynamicConcept (checkAddOperator 1,Impl ()))
printfn "%A" (checkDynamicConcept (checkAddOperator 1,Impl ()))
printfn "%A" (checkAddable 1 2)
printfn "%A" (checkAddable 1.0f 2.0f)
printfn "%A" (checkAddable (AddA()) 2.0f)
