% 取距离最小的前k个作为候选的感染源
k = 100;
nRow = 67;
nCol = 1415;
mat = zeros(nRow,nCol);

for i = 1:nRow
    for j = 1:nCol
        mat(i,j) = Doutput((i-1)*nRow+j,3);
    end
end

argminVec = [];

for i = 1:nRow
    [val, id] = sort(mat(i,:));
    if min(val) < 0.001
        argminVec = [argminVec; id(1:k)];
    end
end

% 做出所有感染源的数量柱状图
figure(1)
hist(argminVec(:), nCol);

% 做出最小距离的分布图
figure(2)
sortMinDist = sort(min(mat, [], 2));
plot(sortMinDist(1:end-100));