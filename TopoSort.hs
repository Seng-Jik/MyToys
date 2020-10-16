-- Topo Sort

import Data.List

data Graph vertex = Graph {
    verticles :: [vertex],
    edges :: [(vertex, vertex)]
}

removeVertex :: Eq vertex => vertex -> Graph vertex -> Graph vertex
removeVertex vertex (Graph v e) = 
    Graph nv ne
    where
        nv = delete vertex v
        ne = filter ((/= vertex) . fst) e
        
topoSorts :: Eq vertex => Graph vertex -> [[vertex]]
topoSorts (Graph v e) =
    topo v vertexNoIndegree
    where
        vertexNoIndegree = v \\ vertexHasIndegree
        vertexHasIndegree = map snd e
        topo [] [] = [[]]
        topo _ [] = []
        topo _ vertexToRemove =
            vertexToRemove >>= (\cur ->
                map (\x -> [cur] ++ x) $ topoSorts $ removeVertex cur (Graph v e))

main :: IO ()
main = 
    let (a, b, c, d, e) = ('a', 'b', 'c', 'd', 'e') in
    let v = [ a, b, c, d, e ] in
    let edge = [ (a, b), (a, c), (a, e), (b, c), (e, d), (c, d) ] in
    let g = Graph v edge in
    output $ topoSorts g
    where
        output :: [[Char]] -> IO ()
        output [] = print "-- End --"
        output (a:x) = 
            do
                print a
                output x
