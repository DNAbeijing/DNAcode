% % 取距离小于的阈值的作为候选
% thres = 0.1;
% nRow = size(unique(matt(:,1)), 1);
% nCol = size(unique(matt(:,2)), 1);
% mat = zeros(nRow,nCol);
% start = min(unique(matt(:,1)));
% 
% for i = 1:nRow
%     for j = 1:nCol
%         mat(i,j) = matt((i-1)*nCol+j,3);
%     end
% end

%%% filter by hand

mat=dataE1(VarName3 > 10,:);
nRow = size(mat,1);

argminVec = [];

for i = 1:nRow
    [val, id] = sort(mat(i,:));
    idn = val<thres;
    argminVec = [argminVec [id(idn); repmat(i,1,sum(idn)); val(val<thres)]];
end

% 做出所有感染源的数量柱状图
figure(1)
hist(argminVec(1,:), nCol);
disp(hist(argminVec(1,:), nCol));