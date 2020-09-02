
type Value = Double
data Stack = Bottom | StackElement Value Stack
data Atom = Operand Value | Operator (Stack -> Stack)

exprs :: [String]
exprs = [ "10 4 3 + 2 * -", "2 3.5 +", "90 34 12 33 55 66 + * - + -", "90 3.8 -", "10 10 10 10 10 sum 4 /", "10 2 ^" ]

twoOperands :: (Value -> Value -> Value) -> Stack -> Stack
twoOperands f (StackElement second (StackElement first s)) = StackElement (f first second) s
twoOperands _ _ = error "No!"

sumi :: Stack -> Stack
sumi Bottom = StackElement 0.0 Bottom
sumi (StackElement v s) = 
    StackElement (v + x) Bottom
    where (StackElement x Bottom) = sumi s

pow :: Value -> Value -> Value
pow _ 0.0 = 1
pow a 1.0 = a
pow a n = a * (pow a $ n - 1.0)

parse :: String -> [Atom]
parse = 
    map f . words
    where f "*" = Operator $ twoOperands (*)
          f "-" = Operator $ twoOperands (-)
          f "+" = Operator $ twoOperands (+)
          f "/" = Operator $ twoOperands (/)
          f "sum" = Operator sumi
          f "^" = Operator $ twoOperands pow
          f x = Operand $ read x
          
run :: [Atom] -> Value
run = 
    getResult . foldl f Bottom
    where f stack (Operand x) = StackElement x stack
          f stack (Operator op) = op stack
          getResult (StackElement x Bottom) = x
          getResult _ = error "Can not get result!"
          
main :: IO ()
main = print $ map (run . parse) exprs
