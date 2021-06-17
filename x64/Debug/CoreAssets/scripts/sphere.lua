function onCollision(obj1,obj2)
    name1 = obj1:GetName()
    name2 = obj2:GetName()

    if name1 == "Sphere" then
        if name2 == "box2" then
            obj2:TriggerDestroy() 
        end
    end
end